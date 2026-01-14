<?php
// Disable all caching
header("Cache-Control: no-store, no-cache, must-revalidate, max-age=0");
header("Cache-Control: post-check=0, pre-check=0", false);
header("Pragma: no-cache");
header("Expires: 0");
header("Expires: Tue, 01 Jan 2000 00:00:00 GMT");
header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
// Autoload Composer dependencies (e.g., Parsedown)
require_once __DIR__ . '/vendor/autoload.php';
define('PLAYER_VERSION', '2.0.2b');

// Force disable HTTPS filtering to allow both HTTP and HTTPS streams
$isSecure = false;
/**
 * Enhanced Icecast JSON Parser for PHP 8.3
 *
 * Handles malformed JSON and intelligently detects online/offline streams
 */
class IcecastParser {
    private ?array $data = null;
    private array $errors = [];
    private array $mounts = [];
    private string $stationName = 'Streaming Server';
    private string $stationDescription = '';
    private string $serverAdmin = '';
    private string $serverLocation = '';
    private string $serverHost = '';
    private string $serverId = '';
    private string $serverStart = '';

    /**
     * Constructor - fetch and parse JSON immediately, with session caching
     *
     * @param string $url The status-json.xsl URL
     */
    public function __construct(string $url) {
        if ($this->fetchAndParseJson($url)) {
            // Filter out offline mounts immediately
            $this->data['icestats']['source'] = array_filter(
                is_array($this->data['icestats']['source']) ? $this->data['icestats']['source'] : [$this->data['icestats']['source']],
                function ($source) {
                    $hasListeners = isset($source['listeners']) && $source['listeners'] > 0;
                    $hasMetadata = !empty($source['server_name']) &&
                        (!empty($source['title']) || !empty($source['yp_currently_playing']));
                    $isAutoDj = isset($source['listenurl']) && stripos($source['listenurl'], 'autodj') !== false;
                    return $hasListeners || ($hasMetadata && $isAutoDj);
                }
            );
        }
    }

    /**
     * Fetch JSON data from the Icecast server and parse it
     *
     * @param string $url The status-json.xsl URL
     * @return bool Success/failure of the operation
     */
    public function fetchAndParseJson(string $url): bool {
        // Set up context with timeout to avoid hanging, and set custom user agent
        $context = stream_context_create([
            'http' => [
                'timeout' => 2,
                'user_agent' => 'CasterClubYPListerAgent/1.1',
                'ignore_errors' => true
            ],
            'ssl' => [
                'verify_peer' => false,
                'verify_peer_name' => false,
                'allow_self_signed' => true
            ]
        ]);

        // Attempt to fetch the data
        $jsonData = @file_get_contents($url, false, $context);

        // Check HTTP response headers for status code
        $headers = $http_response_header ?? [];
        $httpStatus = 0;
        foreach ($headers as $header) {
            if (preg_match('/HTTP\/\d+\.\d+\s+(\d+)/', $header, $matches)) {
                $httpStatus = (int)$matches[1];
                break;
            }
        }
        error_log("HTTP Status: $httpStatus for $url");
        if ($httpStatus !== 200) {
            $this->errors[] = 'HTTP status code ' . $httpStatus . ' for URL: ' . $url;
            error_log("IcecastParser Error: " . end($this->errors));
            return false;
        }

        if ($jsonData === false) {
            $this->errors[] = 'Failed to connect to the Icecast server';
            error_log("IcecastParser Error: " . end($this->errors));
            return false;
        }

        // Clean and fix common JSON issues
        $jsonData = $this->fixJsonStructure($jsonData);

        // Try to parse the JSON
        try {
            $parsedData = json_decode($jsonData, true, 512, JSON_THROW_ON_ERROR);

            // Check for expected structure and extract data
            if (isset($parsedData['icestats'])) {
                $this->data = $parsedData;
                $this->extractData();
                return true;
            } else {
                $this->errors[] = 'Unexpected JSON structure: icestats object not found';
                error_log("IcecastParser Error: " . end($this->errors));
                return false;
            }
        } catch (\JsonException $e) {
            $this->errors[] = 'JSON parse error: ' . $e->getMessage();
            error_log("IcecastParser Error: " . end($this->errors));
            return false;
        }
    }

    /**
     * Fix common issues in Icecast JSON structure
     *
     * @param string $jsonData The original JSON string
     * @return string Fixed JSON string
     */
    private function fixJsonStructure(string $jsonData): string {
        // Replace consecutive commas with a single comma
        $jsonData = preg_replace('/,\s*,/', ',', $jsonData);

        // Fix trailing commas in arrays and objects
        $jsonData = preg_replace('/,\s*\}/', '}', $jsonData);
        $jsonData = preg_replace('/,\s*\]/', ']', $jsonData);

        // Fix missing comma before opening braces
        $jsonData = preg_replace('/\}\s*\{/', '},{', $jsonData);

        // Fix broken objects - this specific pattern appears in your example
        $jsonData = preg_replace('/"\s*,\s*\{/', '"},{"listenurl":', $jsonData);

        // Fix missing braces in malformed objects (common in some Icecast versions)
        $pattern = '/("listenurl":"[^"]+"),(?!\s*"|\s*\})/';
        $jsonData = preg_replace($pattern, '$1}', $jsonData);

        // Fix missing values for listenurl entries
        $pattern = '/"listenurl":"([^"]+)"(?!\s*,\s*"|\s*\})/';
        $jsonData = preg_replace($pattern, '"listenurl":"$1"}', $jsonData);

        // Ensure empty objects have proper closing braces
        $jsonData = preg_replace('/("listenurl":"[^"]+")(?!\s*,|\s*\})/', '$1}', $jsonData);

        return $jsonData;
    }

    /**
     * Extract all data from the parsed JSON
     */
    private function extractData(): void {
        $icestats = $this->data['icestats'];

        // Extract server info
        $this->serverAdmin = $icestats['admin'] ?? '';
        $this->serverHost = $icestats['host'] ?? '';
        $this->serverLocation = $icestats['location'] ?? '';
        $this->serverId = $icestats['server_id'] ?? '';
        $this->serverStart = $icestats['server_start'] ?? '';

        // Extract sources/mounts
        if (isset($icestats['source'])) {
            $sources = $icestats['source'];

            // Handle both array and single object formats
            if (isset($sources['listenurl'])) {
                // Single source object
                $this->processSource($sources);
            } else {
                // Array of source objects
                foreach ($sources as $source) {
                    if (is_array($source)) {
                        $this->processSource($source);
                    }
                }
            }
        }

        // Sort mounts by online status and listeners
        $this->sortMounts();

        // Set station name and description from best available source
        $this->determineStationInfo();
    }

    /**
     * Process a single source/mount and add it to the mounts array
     *
     * @param array $source The source data
     */
    private function processSource(array $source): void {
        $listenurl = $source['listenurl'] ?? '';

        // Skip empty or invalid sources
        if (empty($listenurl)) {
            return;
        }

        // Determine if source is truly online
        // 1. Has listeners > 0
        // 2. Has metadata like title/genre
        // 3. URL matches known active patterns
        $hasListeners = isset($source['listeners']) && $source['listeners'] > 0;
        $hasMetadata = !empty($source['server_name']) &&
            (!empty($source['title']) || !empty($source['yp_currently_playing']));

        // Check if this appears to be an auto-DJ stream (more likely to be active)
        $isAutoDj = stripos($listenurl, 'autodj') !== false;

        // Combined determination of online status with weighted factors
        $online = $hasListeners || ($hasMetadata && $isAutoDj);

        // Get current title from best available source
        $title = $source['yp_currently_playing'] ?? $source['title'] ?? 'No Title Streaming';

        // Add to mounts array with complete data
        $this->mounts[] = [
            'server_name' => $source['server_name'] ?? 'Unknown Station',
            'server_description' => $source['server_description'] ?? 'No Description',
            'title' => $title,
            'genre' => $source['genre'] ?? 'Various',
            'bitrate' => $source['bitrate'] ?? 0,
            'channels' => $source['channels'] ?? 2,
            'samplerate' => $source['samplerate'] ?? 0,
            'listeners' => $source['listeners'] ?? 0,
            'listener_peak' => $source['listener_peak'] ?? 0,
            'listenurl' => $listenurl,
            'type' => $source['server_type'] ?? 'audio/mpeg',
            'stream_start' => $source['stream_start'] ?? '',
            'stream_start_iso8601' => $source['stream_start_iso8601'] ?? '',
            'server_url' => $source['server_url'] ?? '',
            'audio_info' => $source['audio_info'] ?? '',
            'is_autodj' => $isAutoDj,
            'has_metadata' => $hasMetadata,
            'online' => $online
        ];
    }

    /**
     * Sort mounts by online status, listeners, and other criteria
     */
    private function sortMounts(): void {
        // Sort by multiple criteria
        usort($this->mounts, function($a, $b) {
            // First by online status
            if ($a['online'] !== $b['online']) {
                return $b['online'] <=> $a['online']; // Online streams first
            }

            // Then by listener count
            if ($a['listeners'] !== $b['listeners']) {
                return $b['listeners'] <=> $a['listeners']; // Higher listener count first
            }

            // Then by bitrate (higher quality first)
            if ($a['bitrate'] !== $b['bitrate']) {
                return $b['bitrate'] <=> $a['bitrate'];
            }

            // Finally by name
            return strcmp($a['server_name'], $b['server_name']);
        });
    }

    /**
     * Determine the best station name and description from available sources
     */
    private function determineStationInfo(): void {
        // Look for an active mount first
        foreach ($this->mounts as $mount) {
            if ($mount['online']) {
                $this->stationName = $mount['server_name'];
                $this->stationDescription = $mount['server_description'];
                return;
            }
        }

        // If no active mount, use the first available
        if (!empty($this->mounts)) {
            $this->stationName = $this->mounts[0]['server_name'];
            $this->stationDescription = $this->mounts[0]['server_description'];
        }
    }

    /**
     * Get errors encountered during parsing
     *
     * @return array List of error messages
     */
    public function getErrors(): array {
        return $this->errors;
    }

    /**
     * Get all available mounts/sources
     *
     * @return array List of mount points
     */
    public function getMounts(): array {
        return $this->mounts;
    }

    /**
     * Get only online mounts
     *
     * @return array List of online mount points
     */
    public function getOnlineMounts(): array {
        return array_filter($this->mounts, function($mount) {
            return $mount['online'];
        });
    }

    /**
     * Get station name
     *
     * @return string Station name
     */
    public function getStationName(): string {
        return $this->stationName;
    }

    /**
     * Get station description
     *
     * @return string Station description
     */
    public function getStationDescription(): string {
        return $this->stationDescription;
    }

    /**
     * Get server information
     *
     * @return array Server information
     */
    public function getServerInfo(): array {
        return [
            'admin' => $this->serverAdmin,
            'host' => $this->serverHost,
            'location' => $this->serverLocation,
            'server_id' => $this->serverId,
            'server_start' => $this->serverStart
        ];
    }

    /**
     * Get total listener count across all mounts
     *
     * @return int Total listeners
     */
    public function getTotalListeners(): int {
        $total = 0;
        foreach ($this->mounts as $mount) {
            $total += $mount['listeners'];
        }
        return $total;
    }
}

/**
 * Helper function for safe JSON encoding
 *
 * @param mixed $data The data to encode
 * @param string $default Default value if data is empty
 * @return string JSON encoded string
 */
function safeJsonEncode($data, $default = '[]') {
    if (empty($data)) return $default;
    return json_encode($data);
}

/**
 * Modified status-popup.php that uses the enhanced parser
 */

// Start timing for performance measurement
$startTime = microtime(true);
$popupId = uniqid('popup_');
$url = $_GET['url'] ?? '';
$debugMode = isset($_GET['debug']) && ($_GET['debug'] === 'true' || $_GET['debug'] === '1');

// Initialize with defaults
$stationName = 'Streaming Server';
$stationDescription = '';
$mounts = [];
$errors = [];
$serverInfo = [];
$pageLoadTime = 0;

// Process the URL if provided
if (filter_var($url, FILTER_VALIDATE_URL)) {
    // Create parser and fetch data
    $parser = new IcecastParser($url);

    // Check for errors
    $errors = $parser->getErrors();

    if (empty($errors)) {
        // Disable filtering by protocol entirely
        $mounts = $parser->getMounts();
        $stationName = $parser->getStationName();
        $stationDescription = $parser->getStationDescription();
        $serverInfo = $parser->getServerInfo();
    }
}

// Calculate page load time
$pageLoadTime = round(microtime(true) - $startTime, 3);
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge"> <!-- Force Edge/IE compatibility mode -->
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <title>CasterClub AI.Player - <?= htmlspecialchars($stationName) ?></title>
    <!-- Media & Browser Behavior -->
    <meta name="referrer" content="no-referrer"> <!-- Prevent passing referrer -->
    <meta name="theme-color" content="#000000"> <!-- Android toolbar color when popup is opened -->
    <meta name="mobile-web-app-capable" content="yes"> <!-- Allow standalone mode for Chrome on Android -->
    <meta name="apple-mobile-web-app-capable" content="yes"> <!-- Allow standalone mode for Safari on iOS -->
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">

    <!-- Caching and Performance -->
    <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate">
    <meta http-equiv="Pragma" content="no-cache">
    <meta http-equiv="Expires" content="0">

    <!-- Security and Embedding -->
    <meta http-equiv="Content-Security-Policy" content="default-src 'self' data: blob: filesystem: about:; img-src * data: blob:; media-src *; connect-src *; font-src *; style-src 'self' 'unsafe-inline'; script-src 'self' 'unsafe-inline' 'unsafe-eval'">

    <!-- Social and Sharing (optional, if you want nice link previews) -->
    <meta property="og:title" content="CasterClub - Internet Radio Community">
    <meta property="og:description" content="Join the world's leading Internet Radio Community.">
    <meta property="og:type" content="website">
    <meta property="og:image" content="/images/cclub-logo.png"> <!-- Replace with your logo -->
    <meta property="og:url" content="https://yp.casterclub.com/status-popup.php">

    <!-- Favicon (Optional) -->
    <link rel="icon" href="/favicon.ico" type="image/x-icon">
    <style>
        /* Base Styles */
        :root {
            --primary-bg: #000;
            --secondary-bg: #111;
            --panel-bg: #004140;
            --accent-color: #FFFF00;
            --text-color: #CCC;
            --highlight: #0FF;
            --border-color: #004940;
            --online-color: #0F0;
            --offline-color: #F00;
            --border-radius: 10px;
            --tab-active-bg: #006160;
            --tab-inactive-bg: #003130;
        }

        body {
            background: var(--primary-bg);
            color: var(--text-color);
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            padding: 0;
            margin: 0;
            height: 100vh;
            display: flex;
            flex-direction: column;
            font-size: 8px;
        }

        * {
            box-sizing: border-box;
        }

        /* Header - SLIMMED DOWN */
        header {
            background: var(--panel-bg);
            color: var(--accent-color);
            padding: 8px;
            text-align: center;
            border-bottom: 2px solid var(--border-color);
            position: relative;
            display: flex;
            flex-direction: column;
            align-items: center;
            box-shadow: 0 3px 5px rgba(0,0,0,0.3);
            height: auto;
            max-height: 60px;
        }

        h1, .current-title, .stat-value {
            font-size: 10px;
        }
        h1 {
            margin: 0;
            padding: 0;
            font-weight: bold;
            text-shadow: 0 0 5px rgba(255,255,0,0.4);
        }

        header p {
            margin: 0;
            padding: 0;
            font-size: 10pt;
        }

        /* Main Container */
        .player-container {
            max-width: 800px;
            margin: 10px auto;
            background: var(--secondary-bg);
            border-radius: var(--border-radius);
            overflow: hidden;
            box-shadow: 0 10px 25px rgba(0,0,0,0.5);
            border: 1px solid var(--border-color);
            padding: 15px;
            flex: 1;
            display: flex;
            flex-direction: column;
            width: 95%;
        }

        /* On Air Icon */
        .on-air-container {
            position: relative;
            text-align: center;
            margin: 10px 0;
        }

        .on-air {
            display: inline-block;
            background: #500;
            color: #F00;
            padding: 5px 15px;
            border-radius: 5px;
            font-weight: bold;
            box-shadow: 0 0 10px rgba(255,0,0,0.5);
            text-transform: uppercase;
            letter-spacing: 1px;
            border: 1px solid #700;
            animation: pulse-on-air 2s infinite;
        }

        .on-air.live {
            background: #050;
            color: #0F0;
            box-shadow: 0 0 10px rgba(0,255,0,0.5);
            border: 1px solid #070;
        }

        @keyframes pulse-on-air {
            0% { opacity: 0.7; }
            50% { opacity: 1; box-shadow: 0 0 20px rgba(255,0,0,0.8); }
            100% { opacity: 0.7; }
        }

        .on-air.live {
            animation: pulse-on-air-live 2s infinite;
        }

        @keyframes pulse-on-air-live {
            0% { opacity: 0.7; }
            50% { opacity: 1; box-shadow: 0 0 20px rgba(0,255,0,0.8); }
            100% { opacity: 0.7; }
        }

        /* Mount Selector */
        .mount-selector {
            margin-bottom: 15px;
            position: relative;
        }

        select {
            background: var(--panel-bg);
            color: var(--accent-color);
            border: 1px solid var(--border-color);
            padding: 10px 15px;
            border-radius: 20px;
            width: 100%;
            appearance: none;
            cursor: pointer;
            font-size: 16px;
            box-shadow: 0 3px 6px rgba(0,0,0,0.2);
            outline: none;
            transition: all 0.3s ease;
        }

        select:hover {
            background: #005150;
            box-shadow: 0 5px 10px rgba(0,0,0,0.3);
        }

        .mount-selector::after {
            content: '▼';
            position: absolute;
            right: 15px;
            top: 10px;
            color: var(--accent-color);
            pointer-events: none;
        }

        /* Now Playing Section */
        .now-playing {
            background: var(--panel-bg);
            padding: 15px;
            border-radius: var(--border-radius);
            margin-bottom: 15px;
            display: flex;
            flex-direction: column;
            box-shadow: inset 0 0 10px rgba(0,0,0,0.3), 0 3px 6px rgba(0,0,0,0.2);
        }

        .now-playing-header {
            display: flex;
            align-items: center;
            margin-bottom: 10px;
        }

        .playing-indicator {
            display: inline-block;
            animation: pulse 2s infinite;
            color: var(--accent-color);
            font-weight: bold;
            margin-right: 10px;
            font-size: 16px;
        }

        .current-title {
            flex: 1;
            color: var(--highlight);
            font-weight: bold;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
            padding-right: 10px;
            font-size: 14px;
        }

        .buffering-indicator {
            color: var(--accent-color);
            animation: pulse 1s infinite;
            margin-left: auto;
        }

        /* Professional VU Meters */
        .vu-meter-container {
            display: flex;
            flex-direction: column;
            margin-bottom: 10px;
        }

        .vu-meter-stereo {
            display: flex;
            justify-content: space-between;
            gap: 8px;
        }

        .vu-meter {
            position: relative;
            width: 100%;
            height: 25px;
            background: #000;
            border-radius: 3px;
            overflow: hidden;
            box-shadow: inset 0 0 5px rgba(0,0,0,0.7);
            margin-bottom: 2px;
        }

        .vu-meter-label {
            position: absolute;
            left: 5px;
            top: 50%;
            transform: translateY(-50%);
            font-size: 10px;
            color: rgba(255,255,255,0.7);
            z-index: 2;
        }

        .vu-meter-scale {
            position: absolute;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            display: flex;
            align-items: center;
            z-index: 1;
        }

        .vu-meter-tick {
            position: absolute;
            height: 100%;
            width: 1px;
            background-color: rgba(255,255,255,0.2);
        }

        .vu-meter-tick.major {
            background-color: rgba(255,255,255,0.4);
            height: 100%;
        }

        .vu-meter-tick-label {
            position: absolute;
            bottom: 0;
            transform: translateX(-50%);
            font-size: 8px;
            color: rgba(255,255,255,0.5);
        }

        .vu-meter-value {
            position: absolute;
            left: 0;
            top: 0;
            height: 100%;
            width: 0%;
            background: linear-gradient(to right,
            #008800 0%, #00dd00 70%,
            #dddd00 70%, #dddd00 85%,
            #dd0000 85%, #dd0000 100%);
            transition: width 0.1s linear;
            border-radius: 0 2px 2px 0;
            z-index: 0;
        }

        .vu-meter-peak {
            position: absolute;
            top: 0;
            height: 100%;
            width: 2px;
            background-color: #ff0000;
            opacity: 0.8;
            z-index: 3;
            transition: left 0.1s ease-out;
        }

        .vu-rms-value, .vu-peak-value {
            font-size: 10px;
            color: var(--highlight);
            text-align: right;
            margin-top: 2px;
            width: 48px;
            display: inline-block;
        }

        /* Controls Section */
        .controls {
            display: flex;
            flex-wrap: wrap;
            gap: 10px;
            margin-bottom: 15px;
            justify-content: center;
            align-items: center;
        }

        /* Player Buttons */
        .player-buttons {
            display: flex;
            gap: 10px;
        }

        .control-btn {
            background: var(--panel-bg);
            color: var(--accent-color);
            border: none;
            width: 40px;
            height: 40px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            font-size: 18px;
            transition: all 0.2s ease;
            box-shadow: 0 3px 6px rgba(0,0,0,0.3), inset 0 0 10px rgba(0,0,0,0.2);
        }

        .control-btn:hover {
            background: #005150;
            transform: scale(1.05);
            box-shadow: 0 5px 10px rgba(0,0,0,0.4), inset 0 0 15px rgba(0,0,0,0.3);
        }

        .control-btn:active {
            transform: scale(0.95);
        }

        /* Volume Control */
        .volume-control {
            display: flex;
            align-items: center;
            gap: 10px;
            flex: 1;
            min-width: 150px;
            background: rgba(0,0,0,0.3);
            padding: 5px 10px;
            border-radius: 15px;
        }

        .volume-icon {
            color: var(--accent-color);
            font-size: 16px;
        }

        .volume-slider {
            flex: 1;
            -webkit-appearance: none;
            appearance: none;
            height: 6px;
            background: #222;
            border-radius: 3px;
            outline: none;
        }

        .volume-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 16px;
            height: 16px;
            border-radius: 50%;
            background: var(--accent-color);
            cursor: pointer;
            box-shadow: 0 0 5px rgba(255,255,0,0.5);
            transition: all 0.2s ease;
        }

        .volume-slider::-webkit-slider-thumb:hover {
            transform: scale(1.1);
            box-shadow: 0 0 10px rgba(255,255,0,0.7);
        }

        .volume-level {
            color: var(--accent-color);
            min-width: 40px;
            text-align: center;
            font-weight: bold;
            font-size: 12px;
        }

        /* Stream Info */
        .stream-info {
            flex: 1;
            background: rgba(0,0,0,0.2);
            border-radius: var(--border-radius);
            padding: 10px;
            margin-bottom: 15px;
            box-shadow: inset 0 0 10px rgba(0,0,0,0.3);
            overflow: hidden;
            position: relative;
            max-height: 500px;
            display: flex;
            flex-direction: column;
        }

        table {
            width: 100%;
            border-collapse: collapse;
        }

        td {
            padding: 8px;
            border-bottom: 1px solid #333;
        }

        td:first-child {
            font-weight: bold;
            width: 150px;
        }

        .offline-message {
            color: var(--offline-color);
            text-align: center;
            padding: 20px;
            font-weight: bold;
            background: rgba(255,0,0,0.1);
            border-radius: var(--border-radius);
            margin: 15px 0;
            border: 1px solid rgba(255,0,0,0.3);
        }

        /* Tabbed Information Panel */
        .tabbed-container {
            display: flex;
            flex-direction: column;
            height: 100%;
            position: relative;
            margin-bottom: 15px;
            background: rgba(0,0,0,0.2);
            border-radius: var(--border-radius);
            overflow: hidden;
            box-shadow: inset 0 0 10px rgba(0,0,0,0.3);
        }

        .tab-header {
            display: flex;
            background: #222;
            border-bottom: 1px solid #333;
            position: sticky;
            top: 0;
            z-index: 10;
        }

        .tab-button {
            padding: 10px 15px;
            background: var(--tab-inactive-bg);
            color: #AAA;
            border: none;
            cursor: pointer;
            flex-grow: 1;
            text-align: center;
            font-weight: bold;
            transition: all 0.3s ease;
            border-right: 1px solid #111;
        }

        .tab-button:last-child {
            border-right: none;
        }

        .tab-button.active {
            background: var(--tab-active-bg);
            color: var(--accent-color);
            box-shadow: inset 0 -3px 0 var(--accent-color);
        }

        .tab-button:hover:not(.active) {
            background: #004140;
            color: #CCC;
        }

        .tab-content {
            flex: 1;
            overflow-y: auto;
            max-height: 350px;
            padding: 15px;
            display: none;
        }

        .tab-content.active {
            display: block;
        }

        /* Server Info Tab */
        .server-info-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 10px;
        }

        .info-item {
            padding: 5px;
        }

        .info-label {
            font-size: 12px;
            color: #888;
            margin-bottom: 3px;
        }

        .info-value {
            font-size: 14px;
            color: var(--highlight);
        }

        /* Station Details Tab */
        .station-details table {
            margin-top: 10px;
        }

        /* Song History Tab */
        .song-history {
            max-height: 200px;
            overflow-y: auto;
        }

        .song-history-list {
            list-style-type: none;
            padding: 0;
            margin: 0;
        }

        .song-history-item {
            padding: 8px;
            border-bottom: 1px solid #333;
            display: flex;
            align-items: center;
        }

        .song-history-item:last-child {
            border-bottom: none;
        }

        .song-timestamp {
            font-size: 12px;
            color: #888;
            margin-right: 10px;
            min-width: 80px;
        }

        .song-title {
            color: var(--highlight);
            flex: 1;
        }

        /* Listener Stats */
        .listener-stats {
            background: rgba(0,65,64,0.3);
            border-radius: var(--border-radius);
            padding: 10px;
            margin-bottom: 15px;
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            box-shadow: inset 0 0 10px rgba(0,0,0,0.2);
        }

        .stat-item {
            text-align: center;
            padding: 5px 10px;
        }

        .stat-value {
            font-size: 20px;
            font-weight: bold;
            color: var(--highlight);
            display: block;
        }

        .stat-label {
            font-size: 12px;
            color: var(--accent-color);
            text-transform: uppercase;
        }

        /* Status Bar */
        .status-bar {
            background: #111;
            padding: 8px;
            display: flex;
            justify-content: space-between;
            font-size: 12px;
            border-top: 1px solid #222;
            color: #666;
            border-radius: 0 0 var(--border-radius) var(--border-radius);
        }

        /* Status Indicators */
        .online-indicator {
            display: inline-block;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            margin-right: 5px;
        }

        .online-yes {
            background-color: var(--online-color);
            box-shadow: 0 0 5px var(--online-color);
        }

        .online-no {
            background-color: var(--offline-color);
            box-shadow: 0 0 5px var(--offline-color);
        }

        .quality-tag {
            font-size: 10px;
            background: #444;
            padding: 2px 4px;
            border-radius: 3px;
            margin-left: 5px;
        }

        .autodj {
            background: #048;
        }

        .live {
            background: #840;
        }

        /* Buffer Overlay */
        .buffer-overlay {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0, 0, 0, 0.8);
            z-index: 1000;
            justify-content: center;
            align-items: center;
            flex-direction: column;
            backdrop-filter: blur(5px);
        }

        .buffer-message {
            color: var(--accent-color);
            font-size: 18px;
            text-align: center;
            margin-bottom: 20px;
            text-shadow: 0 0 10px rgba(255,255,0,0.5);
        }

        .buffer-spinner {
            width: 60px;
            height: 60px;
            border: 5px solid rgba(0, 65, 64, 0.5);
            border-top: 5px solid var(--accent-color);
            border-bottom: 5px solid var(--accent-color);
            border-radius: 50%;
            animation: spin 1.5s linear infinite;
        }

        .buffer-details {
            color: var(--highlight);
            font-size: 14px;
            margin-top: 15px;
            text-align: center;
        }


        /* Error Display */
        .error {
            background: rgba(255, 0, 0, 0.2);
            color: #FCC;
            padding: 15px;
            margin: 15px auto;
            border: 1px solid #F00;
            border-radius: var(--border-radius);
            max-width: 800px;
            width: 95%;
        }

        /* Animations */
        @keyframes pulse {
            0% { opacity: 0.6; }
            50% { opacity: 1; }
            100% { opacity: 0.6; }
        }

        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }

        /* Media Queries */
        @media (max-width: 768px) {
            .player-container {
                padding: 10px;
            }

            .controls {
                flex-direction: column;
                align-items: stretch;
            }

            .player-buttons {
                justify-content: center;
            }

            .volume-control {
                padding: 5px 10px;
            }

            .status-bar {
                flex-direction: column;
                gap: 5px;
                align-items: center;
            }

            .server-info-grid {
                grid-template-columns: 1fr;
            }

            .tab-button {
                padding: 8px 5px;
                font-size: 12px;
            }
        }
        /* Sliding Now Playing header effect */
        #nowPlayingWrapper {
            display: flex;
            align-items: center;
            position: relative;
            overflow: hidden;
            white-space: nowrap;
            padding-left: 100px; /* create space for the fixed label */
        }

        #nowPlayingLabel {
            font-weight: bold;
            color: var(--accent-color);
            position: absolute;
            left: 0;
            z-index: 3; /* raised above song title */
            background: var(--panel-bg);
            padding-right: 6px;
            font-size: 14px;
        }

        #currentTitle {
            display: inline-block;
            position: relative;
            white-space: nowrap;
            transform: translateX(0);
            transition: none;
        }
    </style>
</head>
<body>

<header>
    <h1><?= htmlspecialchars($stationName) ?></h1>
    <p><?= htmlspecialchars($stationDescription) ?></p>
</header>

<!-- Buffering overlay -->
<div id="bufferOverlay" class="buffer-overlay">
    <div class="buffer-message">BUFFERING STREAM</div>
    <div class="buffer-spinner"></div>
    <div id="bufferDetails" class="buffer-details">Loading high-quality audio stream</div>
</div>

<?php if (!empty($errors)): ?>
    <div class="error">
        <strong>Error:</strong>
        <?php foreach ($errors as $error): ?>
            <div><?= htmlspecialchars($error) ?></div>
        <?php endforeach; ?>
    </div>
<?php endif; ?>

<div class="player-container">
    <?php if (!empty($mounts)): ?>
        <div class="mount-selector">
            <select id="mountSelector" onchange="switchMount()">
                <?php foreach ($mounts as $i => $mount): ?>
                    <?php
                        $statusIcon = $mount['online'] ? '🟢' : '🔴';
                        $streamMode = $mount['is_autodj'] ? 'AutoDJ' : 'Live';
                        $mountName = htmlspecialchars(basename($mount['listenurl']));
                        $bitrate = htmlspecialchars($mount['bitrate']);
                        $type = htmlspecialchars($mount['type']);
                        $listeners = htmlspecialchars($mount['listeners']);
                        $peak = htmlspecialchars($mount['listener_peak']);
                    ?>
                    <option value="<?= $i ?>">
                        <?= $statusIcon ?> <?= $streamMode ?> - Mount <?= $i ?> /<?= $mountName ?> <?= $bitrate ?> kbps <?= $type ?> <?= $listeners ?>/<?= $peak ?>
                    </option>
                <?php endforeach; ?>
            </select>
        </div>

        <!-- On Air Status Indicator -->
        <div class="on-air-container">
            <div id="onAirIndicator" class="on-air">OFFLINE</div>
        </div>

        <div class="now-playing">
            <div class="now-playing-header" id="nowPlayingWrapper">
                <span id="nowPlayingLabel">NOW PLAYING:</span>
                <span id="currentTitle" class="current-title"><?= htmlspecialchars($mounts[0]['title']) ?></span>
            </div>

            <!-- Professional VU Meters -->
            <div class="vu-meter-container">
                <div class="vu-meter-stereo">
                    <div class="vu-meter">
                        <div class="vu-meter-label">L</div>
                        <div id="vuMeterLeftValue" class="vu-meter-value"></div>
                        <div id="vuMeterLeftPeak" class="vu-meter-peak" style="left: 0;"></div>
                        <div class="vu-meter-scale">
                            <?php for($i = 0; $i <= 10; $i++):
                                $position = $i * 10;
                                $majorTick = $i % 2 === 0;
                                ?>
                                <div class="vu-meter-tick <?= $majorTick ? 'major' : '' ?>" style="left: <?= $position ?>%">
                                    <?php if($majorTick): ?>
                                        <span class="vu-meter-tick-label"><?= -30 + ($i * 3) ?></span>
                                    <?php endif; ?>
                                </div>
                            <?php endfor; ?>
                        </div>
                    </div>
                    <div class="vu-rms-value">-∞ dB</div>
                </div>
                <div class="vu-meter-stereo">
                    <div class="vu-meter">
                        <div class="vu-meter-label">R</div>
                        <div id="vuMeterRightValue" class="vu-meter-value"></div>
                        <div id="vuMeterRightPeak" class="vu-meter-peak" style="left: 0;"></div>
                        <div class="vu-meter-scale">
                            <?php for($i = 0; $i <= 10; $i++):
                                $position = $i * 10;
                                $majorTick = $i % 2 === 0;
                                ?>
                                <div class="vu-meter-tick <?= $majorTick ? 'major' : '' ?>" style="left: <?= $position ?>%">
                                    <?php if($majorTick): ?>
                                        <span class="vu-meter-tick-label"><?= -30 + ($i * 3) ?></span>
                                    <?php endif; ?>
                                </div>
                            <?php endfor; ?>
                        </div>
                    </div>
                    <div class="vu-peak-value">-∞ dB</div>
                </div>
            </div>
        </div>

        <!-- Listener Stats -->
        <div class="listener-stats">
            <div class="stat-item">
                <span id="currentListeners" class="stat-value"><?= !empty($mounts) ? $mounts[0]['listeners'] : '0' ?></span>
                <span class="stat-label">Current Listeners</span>
            </div>
            <div class="stat-item">
                <span id="peakListeners" class="stat-value"><?= !empty($mounts) ? $mounts[0]['listener_peak'] : '0' ?></span>
                <span class="stat-label">Peak Listeners</span>
            </div>
            <div class="stat-item">
                <span id="bitrate" class="stat-value"><?= !empty($mounts) ? $mounts[0]['bitrate'] : '0' ?> kbps</span>
                <span class="stat-label">Bitrate</span>
            </div>
        </div>

        <div class="controls">
            <div class="player-buttons">
                <button onclick="playAudio()" class="control-btn">▶️</button>
                <button onclick="pauseAudio()" class="control-btn">⏸️</button>
                <button onclick="stopAudio()" class="control-btn">⏹️</button>
            </div>

            <div class="volume-control">
                <span id="muteToggleIcon" class="volume-icon" onclick="toggleMute()">🔊</span>
                <input type="range" min="0" max="100" value="100" class="volume-slider" id="volumeSlider" oninput="adjustVolume()">
                <span id="volumeLevel" class="volume-level">100%</span>
            </div>

            <button onclick="reloadPage()" class="control-btn">🔄</button>
        </div>

        <?php foreach ($mounts as $i => $mount): ?>
            <div id="playerBlock<?= $i ?>" class="stream-info" style="display:<?= $i == 0 ? 'block' : 'none' ?>">
                <?php if ($mount['online']): ?>
                    <div class="tabbed-container">
                        <div class="tab-header">
                            <button class="tab-button active" onclick="showTab('server-status-<?= $i ?>')">Status</button>
                            <button class="tab-button" onclick="showTab('server-info-<?= $i ?>')">Server Information</button>
                            <button class="tab-button" onclick="showTab('station-details-<?= $i ?>')">Station Details</button>
                            <button class="tab-button" onclick="showTab('song-history-<?= $i ?>')">Song History</button>
                            <button class="tab-button" onclick="showTab('event-log-<?= $i ?>')">Event Log</button>
                            <button class="tab-button" onclick="showTab('about-<?= $i ?>')">About</button>
                        </div>

                        <div id="server-status-<?= $i ?>" class="tab-content active">
                            <table id="mount-status-table">
                                <tr>
                                    <td>Status:</td>
                                    <td>
                                        <span class="online-indicator <?= $mount['online'] ? 'online-yes' : 'online-no' ?>"></span>
                                        <?= $mount['online'] ? 'Online' : 'Offline' ?>
                                        <?php if ($mount['is_autodj']): ?>
                                            <span class="quality-tag autodj">AutoDJ</span>
                                        <?php else: ?>
                                            <span class="quality-tag live">Live</span>
                                        <?php endif; ?>
                                    </td>
                                </tr>
                                <tr><td>Station:</td><td><?= htmlspecialchars($mount['server_name']) ?></td></tr>
                                <tr><td>Description:</td><td><?= htmlspecialchars($mount['server_description']) ?></td></tr>
                                <tr><td>Genre:</td><td><?= htmlspecialchars($mount['genre']) ?></td></tr>
                                <tr><td>Format:</td><td><?= htmlspecialchars($mount['bitrate']) ?> kbps, <?= htmlspecialchars($mount['samplerate'] ? $mount['samplerate']/1000 : '?') ?> kHz, <?= htmlspecialchars($mount['channels']) ?> channels</td></tr>
                                <tr><td>Listeners:</td><td><?= htmlspecialchars($mount['listeners']) ?> current / <?= htmlspecialchars($mount['listener_peak']) ?> peak</td></tr>
                                <tr><td>Currently Playing:</td><td class="nowplaying"><?= htmlspecialchars($mount['title']) ?></td></tr>
                                <tr><td>Stream URL:</td><td><a href="<?= htmlspecialchars($mount['listenurl']) ?>" target="_blank" style="color:var(--highlight);"><?= htmlspecialchars($mount['listenurl']) ?></a></td></tr>
                            </table>
                        </div>

                        <div id="server-info-<?= $i ?>" class="tab-content">
                            <div class="server-info-grid">
                                <div class="info-item">
                                    <div class="info-label">Server Host:</div>
                                    <div class="info-value"><?= htmlspecialchars($serverInfo['host'] ?? 'Unknown') ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Server ID:</div>
                                    <div class="info-value"><?= htmlspecialchars($serverInfo['server_id'] ?? 'Unknown') ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Server Type:</div>
                                    <div class="info-value">Icecast Streaming Server</div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Location:</div>
                                    <div class="info-value"><?= htmlspecialchars($serverInfo['location'] ?? 'Unknown') ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Admin Contact:</div>
                                    <div class="info-value"><?= htmlspecialchars($serverInfo['admin'] ?? 'Unknown') ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Server Start Time:</div>
                                    <div class="info-value"><?= htmlspecialchars($serverInfo['server_start'] ?? 'Unknown') ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Total Listeners:</div>
                                    <div class="info-value"><?= array_sum(array_column($mounts, 'listeners')) ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Total Mounts:</div>
                                    <div class="info-value"><?= count($mounts) ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Active Mounts:</div>
                                    <div class="info-value"><?= count(array_filter($mounts, function($m) { return $m['online']; })) ?></div>
                                </div>
                                <div class="info-item">
                                    <div class="info-label">Server Uptime:</div>
                                    <div class="info-value" id="serverUptime">Calculating...</div>
                                </div>
                            </div>
                        </div>

                        <div id="station-details-<?= $i ?>" class="tab-content">
                            <div class="station-details">
                                <div id="stationDetailsContent<?= $i ?>">
                                    <h3><?= htmlspecialchars($mount['server_name']) ?></h3>
                                    <p><?= htmlspecialchars($mount['server_description']) ?></p>
                                    <table>
                                        <tr>
                                            <td>Stream Type:</td>
                                            <td><?= htmlspecialchars($mount['type']) ?></td>
                                        </tr>
                                        <tr>
                                            <td>Bitrate:</td>
                                            <td><?= htmlspecialchars($mount['bitrate']) ?> kbps</td>
                                        </tr>
                                        <tr>
                                            <td>Sample Rate:</td>
                                            <td><?= htmlspecialchars($mount['samplerate'] ? $mount['samplerate']/1000 : '?') ?> kHz</td>
                                        </tr>
                                        <tr>
                                            <td>Channels:</td>
                                            <td><?= htmlspecialchars($mount['channels']) ?><?= $mount['channels'] == 2 ? ' (Stereo)' : ' (Mono)' ?></td>
                                        </tr>
                                        <tr>
                                            <td>Genre:</td>
                                            <td><?= htmlspecialchars($mount['genre']) ?></td>
                                        </tr>
                                        <tr>
                                            <td>Stream Mode:</td>
                                            <td><?= $mount['is_autodj'] ? 'AutoDJ (Automated)' : 'Live Broadcast' ?></td>
                                        </tr>
                                        <tr>
                                            <td>Current Listeners:</td>
                                            <td><?= htmlspecialchars($mount['listeners']) ?></td>
                                        </tr>
                                        <tr>
                                            <td>Peak Listeners:</td>
                                            <td><?= htmlspecialchars($mount['listener_peak']) ?></td>
                                        </tr>
                                        <tr>
                                            <td>Stream Started:</td>
                                            <td><?= htmlspecialchars($mount['stream_start'] ?? 'Unknown') ?></td>
                                        </tr>
                                    </table>
                                    <h4>Stream Technical Information</h4>
                                    <table>
                                        <tr>
                                            <td>Mount Point:</td>
                                            <td><?= htmlspecialchars(basename($mount['listenurl'])) ?></td>
                                        </tr>
                                        <tr>
                                            <td>Full URL:</td>
                                            <td><a href="<?= htmlspecialchars($mount['listenurl']) ?>" target="_blank" style="color:var(--highlight);"><?= htmlspecialchars($mount['listenurl']) ?></a></td>
                                        </tr>
                                        <tr>
                                            <td>Audio Info:</td>
                                            <td><?= htmlspecialchars($mount['audio_info'] ?? 'Not provided') ?></td>
                                        </tr>
                                    </table>
                                </div>
                            </div>
                        </div>

                        <div id="song-history-<?= $i ?>" class="tab-content">
                            <ul style="list-style-type: none; padding: 0; font-family: monospace;">
                                <?php
                                $entry = date('d/m/Y - h:i:s A') . ' - ' . ($mount['server_name'] ?? '') . ': ' . ($mount['title'] ?? 'No Title Streaming');
                                $songHistory = [$entry];
                                $index = 1;
                                foreach ($songHistory as $entry):
                                    preg_match('/^(\d{2}\/\d{2}\/\d{4} - \d{2}:\d{2}:\d{2} [APM]{2}) - (.*)$/', $entry, $parts);
                                    $timestamp = $parts[1] ?? '';
                                    $stationTitle = $parts[2] ?? $entry;
                                ?>
                                    <li style="padding: 4px 0; border-bottom: 1px solid #333; font-size:10px;">
                                        <?= str_pad($index++, 2, '0', STR_PAD_LEFT) ?>.
                                        <strong><?= htmlspecialchars($timestamp) ?></strong> - <?= htmlspecialchars($stationTitle) ?>
                                    </li>
                                <?php endforeach; ?>
                            </ul>
                        </div>

                        <div id="event-log-<?= $i ?>" class="tab-content">
                            <div id="eventLogOutput<?= $i ?>" style="font-size:8px; padding:5px; background:#000; color:#78D691; font-family:monospace; height:120px; overflow-y:auto;"></div>
                        </div>
                        <div id="about-<?= $i ?>" class="tab-content">
                            <div style="font-size:12px; line-height:1.4;">
                                <h2 style="color:#FFFF00;">CasterClub AI.Player <?= PLAYER_VERSION ?></h2>
                                <p><strong>Build Date:</strong> <?= date('Y-m-d H:i:s') ?></p>
                                <p><strong>Credits:</strong> Developed by David St John •
                                    <a href="https://github.com/casterclub" target="_blank" style="color:var(--highlight);">github.com/casterclub</a>
                                </p>
                                <p><strong>Email:</strong>
                                    <a href="mailto:git@casterclub.com" style="color:var(--highlight);">git@casterclub.com</a>
                                </p>
                                <p><strong>Social:</strong></p>
                                <ul style="margin-left: 20px;">
                                    <li>📘 Facebook:
                                        <a href="https://www.facebook.com/officialcasterclub" target="_blank" style="color:var(--highlight);">facebook.com/officialcasterclub</a>
                                    </li>
                                    <li>📸 Instagram:
                                        <a href="https://www.instagram.com/casterclub.ai/" target="_blank" style="color:var(--highlight);">@casterclub.ai</a>
                                    </li>
                                </ul>
                                <p><strong>Powered by:</strong>
                                    <a href="https://www.instagram.com/mediacast1.ai" target="_blank" style="color:var(--highlight);">MediaCast1</a>
                                </p>
                                <ul style="margin-left: 20px;">
                                    <li>📘 Facebook:
                                        <a href="https://www.facebook.com/officialmediacast1" target="_blank" style="color:var(--highlight);">facebook.com/officialmediacast1</a>
                                    </li>
                                    <li>📸 Instagram:
                                        <a href="https://www.instagram.com/mediacast1.ai/" target="_blank" style="color:var(--highlight);">@mediacast1.ai</a>
                                    </li>
                                </ul>
                                <hr>
                                <?php
                                $readmePath = __DIR__ . '/PLAYER_README.md';
                                if (file_exists($readmePath)) {
                                    $parsedown = new Parsedown();
                                    $parsedown->setSafeMode(true);
                                    echo '<div class="markdown-body" style="font-size:11px; color:#ccc; max-height:300px; overflow-y:auto;">';
                                    echo $parsedown->text(file_get_contents($readmePath));
                                    echo '</div>';
                                } else {
                                    echo '<p style="color:red;">PLAYER_README.md not found.</p>';
                                }
                                ?>
                            </div>
                        </div>
                    </div>
                    <!-- Audio element with crossorigin attribute -->
                    <audio id="audio<?= $i ?>" preload="none" crossorigin="anonymous">
                        <source src="<?= htmlspecialchars($mount['listenurl']) ?>" type="<?= htmlspecialchars($mount['type']) ?>">
                        Your browser does not support HTML5 audio.
                    </audio>
                <?php else: ?>
                    <div class="offline-message">⚠️ Stream Offline or Unavailable 🔇</div>
                <?php endif; ?>
            </div>
        <?php endforeach; ?>


        <div class="status-bar">
            <div>Server: <?= htmlspecialchars($serverInfo['host'] ?? 'Unknown') ?></div>
            <div>Player Version: <?= PLAYER_VERSION ?> <span onclick="toggleAboutModal()" style="cursor:pointer; color:var(--accent-color);" title="About this player">ℹ️</span></div>
            <div>Page loaded in <?= $pageLoadTime ?> seconds</div>
        </div>
    <?php else: ?>
        <div class="no-streams">
            <p>No streams available. Please check the URL and try again.</p>
            <?php if (!empty($url)): ?>
                <p>Attempted to connect to: <?= htmlspecialchars($url) ?></p>
            <?php else: ?>
                <p>No URL provided. Please add a "url" parameter to your request.</p>
            <?php endif; ?>
        </div>
    <?php endif; ?>
<!-- About Modal -->
<div id="aboutModal" style="display:none; position:fixed; top:50%; left:50%; transform:translate(-50%,-50%); background:#111; color:#ccc; padding:20px; border:1px solid #333; border-radius:10px; z-index:9999; width:90%; max-width:500px; font-size:12px;">
  <h2 style="color:#FFFF00;">CasterClub AI.Player v2.0.1b</h2>
  <p><strong>Build Date:</strong> <?= date('Y-m-d H:i:s') ?></p>
  <p><strong>Credits:</strong> Developed by David St John • CasterClub.com</p>
  <h3 style="margin-top:10px;">Release Notes:</h3>
  <ul>
    <li>🎵 Smoother title transitions with fade effect</li>
    <li>✅ Fixed toggling issue between now playing titles</li>
    <li>🚫 Added cache-busting to prevent stale metadata</li>
    <li>📜 Event log now shows tab-specific debug output</li>
  </ul>
  <button onclick="toggleAboutModal()" style="margin-top:10px; padding:5px 10px;">Close</button>
</div>


<script>
    // Safely get PHP variables with default values
    const mounts = <?= !empty($mounts) ? json_encode($mounts) : '[]' ?>;
    const debugMode = <?= $debugMode ? 'true' : 'false' ?>;
    const reloadUrl = '<?= urlencode($url) ?>';
    const initialStationName = '<?= htmlspecialchars(addslashes($stationName)) ?>';

    // Global variables
    let currentAudio = null;
    let currentBlock = document.getElementById('playerBlock0');
    let currentIndex = 0;
    let isPlaying = false;
    let isBuffering = false;
    let bufferTimeout = null;

    // Audio analysis variables
    let audioContext = null;
    let audioSourceNode = null;
    let analyserNode = null;
    let gainNode = null;
    let scriptProcessorNode = null;

    // VU meter variables
    let vuMeterInterval = null;
    let peakLeft = -100;
    let peakRight = -100;
    let peakHoldTime = 2000; // How long to hold peak value in ms
    let peakHoldTimeoutLeft = null;
    let peakHoldTimeoutRight = null;

    // Song history tracking
    let songHistory = [];
    let currentSongTitle = '';
    let songCheckInterval = null;
    let titleCheckInterval = null;

    // Server info variables
    let serverStartTime = <?= json_encode($serverInfo['server_start'] ?? '') ?>;

    // Initialize the player
    window.onload = function() {
        // Get first audio element if it exists
        currentAudio = document.getElementById('audio0');
        if (currentAudio) {
            setupAudioEvents(currentAudio);
            logDebug('🎧 Audio events set up successfully');
        }

        updateNowPlaying();
        logDebug('Updating now playing information');
        logDebug(`[updateNowPlaying] Refreshing now playing display`);
        updatePlayingStatus();
        logDebug('Updating playing status');
        updateOnAirIndicator();
        logDebug('Updating on air indicator');
        updateListenerStats();
        logDebug('Updating listener stats');
        updateStationDetails();
        logDebug('Updating station details');

        // Set initial volume
        document.getElementById('volumeSlider').value = 100;
        document.getElementById('volumeLevel').textContent = '100%';


        logDebug('Player initialized', 'info');

        // Test browser audio capabilities
        testBrowserCapabilities();

        // Hide buffering indicator initially
        document.getElementById('bufferingIndicator').style.display = 'none';

        // Start server uptime counter
        updateServerUptime();
        setInterval(updateServerUptime, 1000);

        // Start song history tracker
        startSongHistoryTracker();

        // Start title streaming with periodic checks
        startTitleStreaming();

        // Start polling status-json.xsl for now playing updates
        if (mounts.length > 0) {
            startNowPlayingPolling(mounts[0].listenurl);
        }
    };


    // --- Enhanced Now Playing Title Animation and Polling ---
    // Persistent variable to track last known song title
    let lastKnownTitle = '';

    function updateNowPlayingTitle(newTitle) {
        const titleElement = document.getElementById('currentTitle');
        if (!titleElement) return;
        // Only update if title actually changed
        if (newTitle !== lastKnownTitle) {
            lastKnownTitle = newTitle;
            // Update the text content
            titleElement.textContent = newTitle;
            // Reset scrolling animation
            titleElement.style.transition = 'none';
            titleElement.style.transform = 'translateX(0)';
            void titleElement.offsetWidth; // force reflow
            titleElement.style.transition = 'transform 10s linear';
            titleElement.style.transform = 'translateX(-100%)';
            // Update window/document title
            document.title = '🎵 ' + newTitle;
        }
    }

    // Poll status-json.xsl for now playing updates
    setInterval(() => {
        if (!mounts || mounts.length === 0) return;
        const selectedMount = document.getElementById("mountSelector") ? document.getElementById("mountSelector").value : 0;
        const idx = typeof selectedMount === 'string' ? parseInt(selectedMount, 10) : 0;
        const mount = mounts[idx];
        if (!mount) return;
        fetch(`status-json.xsl?mount=${encodeURIComponent(mount.listenurl)}`)
            .then(response => response.json())
            .then(data => {
                if (data && data.icestats && data.icestats.source) {
                    const sources = Array.isArray(data.icestats.source) ? data.icestats.source : [data.icestats.source];
                    const source = sources.find(src => src.listenurl === mount.listenurl);
                    if (source && (source.title || source.yp_currently_playing)) {
                        const songTitle = source.yp_currently_playing || source.title;
                        // Only update if changed
                        if (songTitle !== lastKnownTitle) {
                            updateNowPlayingTitle(songTitle);
                        }
                    }
                }
            })
            .catch(err => {
                if (debugMode) console.error('Now Playing fetch failed', err);
            });
    }, 5000);

    // Start title streaming feature - updates document title with current song
    function startTitleStreaming() {
        // Set initial title
        updateDocumentTitle();

        // Check for title changes every 2 seconds
        titleCheckInterval = setInterval(function() {
            if (isPlaying) {
                updateDocumentTitle();
            }
        }, 2000);
    }

    // Update the document title with current song
    function updateDocumentTitle() {
        if (!mounts || mounts.length === 0) return;

        const stationName = mounts[currentIndex]?.server_name || initialStationName;
        const songTitle = document.getElementById('currentTitle')?.textContent || 'No Title';

        // Format: CasterClub AI.Player - Station Name - Current Song
        document.title = `CasterClub AI.Player - ${stationName} - ${songTitle}`;
    }

    // Tab switching function
    function showTab(tabId) {
        logDebug(`[showTab] 📑 Tab selected: ${tabId}`);

        // Hide all tabs
        document.querySelectorAll('.tab-content').forEach(tab => {
            tab.classList.remove('active');
        });

        // Deactivate all tab buttons
        document.querySelectorAll('.tab-button').forEach(button => {
            button.classList.remove('active');
        });

        // Show selected tab
        const selectedTab = document.getElementById(tabId);
        if (selectedTab) {
            selectedTab.classList.add('active');
        }

        // Activate selected tab button
        const selectedButton = Array.from(document.querySelectorAll('.tab-button')).find(
            button => button.onclick.toString().includes(tabId)
        );
        if (selectedButton) {
            selectedButton.classList.add('active');
        }

        // Update dynamic content when switching tabs
        if (tabId === 'station-details') {
            updateStationDetails();
        }
    }

    // Update server uptime display
    function updateServerUptime() {
        const uptimeElement = document.getElementById('serverUptime');
        if (!uptimeElement || !serverStartTime) return;

        try {
            // Parse server start time (assuming ISO format or similar)
            const startDate = new Date(serverStartTime);
            if (isNaN(startDate.getTime())) {
                uptimeElement.textContent = 'Unknown';
                return;
            }

            // Calculate difference
            const now = new Date();
            const diff = now - startDate;

            // Convert to days, hours, minutes, seconds
            const days = Math.floor(diff / (1000 * 60 * 60 * 24));
            const hours = Math.floor((diff % (1000 * 60 * 60 * 24)) / (1000 * 60 * 60));
            const minutes = Math.floor((diff % (1000 * 60 * 60)) / (1000 * 60));
            const seconds = Math.floor((diff % (1000 * 60)) / 1000);

            // Format display
            let uptime = '';
            if (days > 0) uptime += `${days}d `;
            uptime += `${hours}h ${minutes}m ${seconds}s`;

            uptimeElement.textContent = uptime;
        } catch (e) {
            uptimeElement.textContent = 'Error calculating';
            logDebug(`Error calculating uptime: ${e.message}`, 'error');
        }
    }

    // Start song history tracker
    function startSongHistoryTracker() {
        // Check for title changes every 5 seconds
        songCheckInterval = setInterval(checkForTitleChanges, 5000);

        // Initialize with current song if playing
        if (isPlaying) {
            const title = document.getElementById('currentTitle').textContent;
            if (title && title !== 'No title available') {
                addSongToHistory(title);
            }
        }
    }

    // Check for title changes
    function checkForTitleChanges() {
        if (!isPlaying) return;

        const title = document.getElementById('currentTitle')?.textContent;
        if (!title || title === 'No title available') return;

        // Check if title has changed
        if (title !== currentSongTitle) {
            addSongToHistory(title);
            updateDocumentTitle(); // Update window title when song changes
        }
    }

    // Add song to history
    function addSongToHistory(title) {
        // Update current title
        currentSongTitle = title;

        // Get current time
        const now = new Date();
        const timestamp = now.toLocaleTimeString();

        // Create song history entry
        const songEntry = {
            title: title,
            timestamp: timestamp,
            datetime: now
        };

        // Add to history (limit to 50 entries)
        songHistory.unshift(songEntry);
        if (songHistory.length > 50) {
            songHistory.pop();
        }

        // Update the display
        updateSongHistoryDisplay();
    }

    // Update song history display
    function updateSongHistoryDisplay() {
        const historyList = document.getElementById('songHistoryList');
        if (!historyList) return;

        // Clear existing content
        historyList.innerHTML = '';

        // Add history items
        if (songHistory.length === 0) {
            const emptyItem = document.createElement('li');
            emptyItem.className = 'song-history-item';
            emptyItem.textContent = 'No song history recorded yet.';
            historyList.appendChild(emptyItem);
        } else {
            songHistory.forEach(song => {
                const item = document.createElement('li');
                item.className = 'song-history-item';

                const timestamp = document.createElement('span');
                timestamp.className = 'song-timestamp';
                timestamp.textContent = song.timestamp;

                const title = document.createElement('span');
                title.className = 'song-title';
                title.textContent = song.title;

                item.appendChild(timestamp);
                item.appendChild(title);
                historyList.appendChild(item);
            });
        }
    }

    // Update station details tab content
    function updateStationDetails() {
        const detailsContent = document.getElementById('stationDetailsContent');
        if (!detailsContent) return;

        if (!mounts || mounts.length === 0 || currentIndex === null) {
            detailsContent.innerHTML = '<p>No station details available.</p>';
            return;
        }

        const mount = mounts[currentIndex];
        if (!mount) {
            detailsContent.innerHTML = '<p>Selected mount point not found.</p>';
            return;
        }

        // Create detailed station information
        let html = `
            <h3>${mount.server_name}</h3>
            <p>${mount.server_description}</p>

            <table>
                <tr>
                    <td>Stream Type:</td>
                    <td>${mount.type}</td>
                </tr>
                <tr>
                    <td>Bitrate:</td>
                    <td>${mount.bitrate} kbps</td>
                </tr>
                <tr>
                    <td>Sample Rate:</td>
                    <td>${mount.samplerate ? mount.samplerate/1000 : '?'} kHz</td>
                </tr>
                <tr>
                    <td>Channels:</td>
                    <td>${mount.channels} (${mount.channels === 2 ? 'Stereo' : 'Mono'})</td>
                </tr>
                <tr>
                    <td>Genre:</td>
                    <td>${mount.genre}</td>
                </tr>
                <tr>
                    <td>Stream Mode:</td>
                    <td>${mount.is_autodj ? 'AutoDJ (Automated)' : 'Live Broadcast'}</td>
                </tr>
                <tr>
                    <td>Current Listeners:</td>
                    <td>${mount.listeners}</td>
                </tr>
                <tr>
                    <td>Peak Listeners:</td>
                    <td>${mount.listener_peak}</td>
                </tr>
                <tr>
                    <td>Stream Started:</td>
                    <td>${mount.stream_start || 'Unknown'}</td>
                </tr>
            </table>

            <h4>Stream Technical Information</h4>
            <table>
                <tr>
                    <td>Mount Point:</td>
                    <td>${mount.listenurl.split('/').pop()}</td>
                </tr>
                <tr>
                    <td>Full URL:</td>
                    <td><a href="${mount.listenurl}" target="_blank" style="color:var(--highlight);">${mount.listenurl}</a></td>
                </tr>
                <tr>
                    <td>Audio Info:</td>
                    <td>${mount.audio_info || 'Not provided'}</td>
                </tr>
            </table>
        `;

        detailsContent.innerHTML = html;
    }

    // Mute toggle functionality for all audio elements
    let isMuted = false;
    function toggleMute() {
        isMuted = !isMuted;
        const volumeSlider = document.getElementById('volumeSlider');
        const volumeLevel = document.getElementById('volumeLevel');
        const muteIcon = document.getElementById('muteToggleIcon');
        const audioElements = document.querySelectorAll('audio');

        audioElements.forEach(audio => {
            audio.volume = isMuted ? 0 : 1;
        });

        volumeSlider.value = isMuted ? 0 : 100;
        volumeLevel.textContent = isMuted ? '0%' : '100%';
        muteIcon.textContent = isMuted ? '🔈' : '🔊';
    }

    // Test browser audio capabilities
    function testBrowserCapabilities() {
        logDebug(`Browser: ${navigator.userAgent}`, 'info');

        // Test Audio API
        if (typeof Audio !== "undefined") {
            logDebug("HTML5 Audio API supported", 'info');
        } else {
            logDebug("HTML5 Audio API NOT supported", 'warning');
        }

        // Test Web Audio API
        if (window.AudioContext || window.webkitAudioContext) {
            logDebug("Web Audio API supported", 'info');
        } else {
            logDebug("Web Audio API NOT supported", 'warning');
        }

        // Test if browser can create AnalyserNode
        try {
            const AudioContextClass = window.AudioContext || window.webkitAudioContext;
            const tempContext = new AudioContextClass();
            const tempAnalyser = tempContext.createAnalyser();
            logDebug("AnalyserNode creation supported", 'info');
            tempContext.close();
        } catch (e) {
            logDebug(`AnalyserNode creation failed: ${e.message}`, 'error');
        }

        // Get mount info
        if (mounts && mounts.length > 0) {
            logDebug(`Mount count: ${mounts.length}`, 'info');
            logDebug(`First mount type: ${mounts[0].type}`, 'info');
            logDebug(`First mount URL: ${mounts[0].listenurl.substring(0, 30)}...`, 'info');
        }
    }

    // Logging function for debug information
    function logDebug(message, level = 'debug') {
        const logArea = document.getElementById(`eventLogOutput${currentIndex}`);
        const timestamp = new Date().toLocaleTimeString();
        if (logArea) {
            const entry = document.createElement('div');
            entry.textContent = `[${timestamp}] ${message}`;
            logArea.appendChild(entry);
            logArea.scrollTop = logArea.scrollHeight;
        }
        console.log(`[${timestamp}] ${message}`);
    }

    // Toggle debug modal visibility
    function toggleDebugModal() {
        const modal = document.getElementById('debugModal');
        if (modal.style.display === 'block') {
            modal.style.display = 'none';
        } else {
            modal.style.display = 'block';
        }
    }

    // Set up audio event listeners
    function setupAudioEvents(audioElement) {
        if (!audioElement) {
            logDebug('No audio element provided to setupAudioEvents', 'error');
            return;
        }

        logDebug('Setting up audio events', 'info');

        // Buffering events
        audioElement.addEventListener('waiting', function() {
            logDebug('Audio waiting event', 'info');
            showBuffering(true);
        });

        audioElement.addEventListener('playing', function() {
            logDebug('Audio playing event', 'info');
            showBuffering(false);
            isPlaying = true;
            updatePlayingStatus();
            updateOnAirIndicator();
            updateDocumentTitle();

            // Add current song to history
            const title = document.getElementById('currentTitle').textContent;
            if (title && title !== 'No title available') {
                addSongToHistory(title);
            }

            // Set up audio analysis for real VU meters
            setupAudioAnalysis();
        });

        audioElement.addEventListener('play', function() {
            logDebug('Audio play event', 'info');
            showBuffering(true);

            setTimeout(function() {
                if (!isPlaying) {
                    const bufferDetails = document.getElementById('bufferDetails');
                    if (bufferDetails) {
                        bufferDetails.textContent = "Loading " + getCurrentMountBitrate() + " kbps stream... please wait";
                    }
                }
            }, 1000);

            isPlaying = true;
            updatePlayingStatus();
            updateOnAirIndicator();
            updateDocumentTitle();
        });

        audioElement.addEventListener('pause', function() {
            logDebug('Audio pause event', 'info');
            isPlaying = false;
            showBuffering(false);
            updatePlayingStatus();
            updateOnAirIndicator();
            stopVUMeterAnimation();
        });

        audioElement.addEventListener('ended', function() {
            logDebug('Audio ended event', 'info');
            isPlaying = false;
            showBuffering(false);
            updatePlayingStatus();
            updateOnAirIndicator();
            stopVUMeterAnimation();
        });

        // Handle metadata updates for title streaming
        audioElement.addEventListener('loadedmetadata', function() {
            logDebug('Audio loaded metadata', 'info');
            updateDocumentTitle();
        });

        // Track title changes when they happen
        // Some streams update metadata without stopping
        audioElement.addEventListener('durationchange', function() {
            // Duration changes can sometimes indicate metadata updates
            updateNowPlaying();
            updateDocumentTitle();
        });

        // Handle errors
        audioElement.addEventListener('error', function(e) {
            let errorMessage = "Unknown error";

            // Get detailed error information if available
            if (audioElement.error) {
                const errorCodes = ['MEDIA_ERR_ABORTED', 'MEDIA_ERR_NETWORK', 'MEDIA_ERR_DECODE', 'MEDIA_ERR_SRC_NOT_SUPPORTED'];
                if (audioElement.error.code >= 1 && audioElement.error.code <= 4) {
                    errorMessage = errorCodes[audioElement.error.code - 1];
                }
                if (audioElement.error.message) {
                    errorMessage += `: ${audioElement.error.message}`;
                }
            }

            logDebug(`Audio error on mount ${currentIndex}: ${errorMessage}`, 'error');

            isPlaying = false;
            showBuffering(false);
            updatePlayingStatus();
            updateOnAirIndicator();
            stopVUMeterAnimation();

            // Show error message
            const playerBlock = document.getElementById('playerBlock' + currentIndex);
            if (playerBlock) {
                let errorMsg = document.createElement('div');
                errorMsg.className = 'offline-message';
                errorMsg.textContent = `Error playing this stream: ${errorMessage}`;

                // Insert at the end of player block
                playerBlock.appendChild(errorMsg);

                // Hide the error after 5 seconds
                setTimeout(function() {
                    if (errorMsg.parentNode) {
                        errorMsg.parentNode.removeChild(errorMsg);
                    }
                }, 5000);
            }
        });

        // Stalled detection
        audioElement.addEventListener('stalled', function() {
            logDebug('Audio stalled event', 'info');
            showBuffering(true);
        });

        // Data loading events for debugging
        audioElement.addEventListener('canplay', function() {
            logDebug('Audio can play event', 'info');
        });

        // Log additional debugging info
        const debugEvents = ['loadstart', 'loadeddata', 'canplaythrough', 'progress', 'timeupdate'];
        debugEvents.forEach(event => {
            audioElement.addEventListener(event, function() {
                logDebug(`Audio ${event} event`, 'debug');
            });
        });
    }

    // Initialize audio analysis system
    function setupAudioAnalysis() {
        // Clean up any existing audio processing
        stopVUMeterAnimation();
        cleanupAudioAnalysis();

        try {
            logDebug('Setting up audio analysis system', 'info');

            // Create new AudioContext
            const AudioContextClass = window.AudioContext || window.webkitAudioContext;
            if (!AudioContextClass) {
                throw new Error('Web Audio API not supported');
            }

            audioContext = new AudioContextClass();
            logDebug(`Audio context created, state: ${audioContext.state}`, 'info');

            // Resume context if suspended
            if (audioContext.state === 'suspended') {
                audioContext.resume()
                    .then(() => logDebug('Audio context resumed', 'info'))
                    .catch(err => logDebug(`Failed to resume context: ${err}`, 'error'));
            }

            // Create analyzer with appropriate settings for audio level monitoring
            analyserNode = audioContext.createAnalyser();
            analyserNode.fftSize = 2048; // Larger FFT for more detailed analysis
            analyserNode.smoothingTimeConstant = 0.5; // Lower values = faster VU response

            // Create gain node for volume control
            gainNode = audioContext.createGain();
            gainNode.gain.value = document.getElementById('volumeSlider').value / 100;

            // Connect audio element to Web Audio API nodes
            try {
                audioSourceNode = audioContext.createMediaElementSource(currentAudio);

                // Chain the nodes: source -> gain -> analyser -> destination
                audioSourceNode.connect(gainNode);
                gainNode.connect(analyserNode);
                analyserNode.connect(audioContext.destination);

                logDebug('Audio nodes connected successfully', 'info');

                // Create script processor for precise level monitoring
                // This is deprecated but still widely supported and better for accurate VU meters
                const bufferSize = 4096;
                scriptProcessorNode = audioContext.createScriptProcessor(bufferSize, 2, 2);

                analyserNode.connect(scriptProcessorNode);
                scriptProcessorNode.connect(audioContext.destination);

                // Set up RMS calculation
                scriptProcessorNode.onaudioprocess = calculateAudioLevels;

                logDebug('Audio processor initialized for real-time level analysis', 'info');

                // Start VU meter animation
                startVUMeterAnimation();
            } catch (e) {
                logDebug(`Error connecting audio nodes: ${e.message}`, 'error');
                throw e;
            }
        } catch (e) {
            logDebug(`Audio analysis setup failed: ${e.message}`, 'error');
            // Fall back to simpler VU meter implementation
            startSimpleVUMeterAnimation();
        }
    }

    // Calculate audio levels using RMS (Root Mean Square) for more accurate VU meters
    function calculateAudioLevels(event) {
        if (!isPlaying) return;

        try {
            // Get audio data from left and right channels
            const inputL = event.inputBuffer.getChannelData(0);
            const inputR = event.inputBuffer.getChannelData(1);

            // Calculate RMS values (true audio level measurement)
            let rmsL = 0;
            let rmsR = 0;

            // Process all samples in the buffer
            for (let i = 0; i < inputL.length; i++) {
                // Square each sample value (audio is in range -1.0 to 1.0)
                rmsL += inputL[i] * inputL[i];
                rmsR += inputR[i] * inputR[i];
            }

            // Finish RMS calculation
            rmsL = Math.sqrt(rmsL / inputL.length);
            rmsR = Math.sqrt(rmsR / inputR.length);

            // Get current volume setting to adjust sensitivity
            const volumeSlider = document.getElementById('volumeSlider');
            const volumeLevel = volumeSlider ? volumeSlider.value / 100 : 1.0;

            // Apply volume compensation factor - makes meters more sensitive at lower volumes
            // When volume is low, we boost the meter display to keep it visually responsive
            let compensationFactor = 1.0;
            if (volumeLevel < 0.8) {
                // Increase compensation as volume decreases
                // Formula creates a curve that boosts low volumes more aggressively
                compensationFactor = Math.pow(1 / Math.max(0.1, volumeLevel), 0.5);

                // Limit maximum compensation
                compensationFactor = Math.min(compensationFactor, 3.0);
            }

            // Apply compensation to RMS values
            const adjustedRmsL = rmsL * compensationFactor;
            const adjustedRmsR = rmsR * compensationFactor;

            // Convert to dB scale (audio engineering standard)
            // 20 * log10(rms) is the standard formula for converting amplitude to dB
            const dbL = 20 * Math.log10(adjustedRmsL || 0.0000001); // Avoid log(0)
            const dbR = 20 * Math.log10(adjustedRmsR || 0.0000001);

            // Update VU meter directly from audio processor for most accurate response
            updateVUMeter(dbL, dbR);
        } catch (e) {
            logDebug(`Error in audio processing: ${e.message}`, 'error');
        }
    }

    // Start VU meter animation based on audio levels
    function startVUMeterAnimation() {
        // If script processor is working, we don't need this interval
        if (!scriptProcessorNode) {
            vuMeterInterval = setInterval(function() {
                if (!isPlaying || !analyserNode) return;

                try {
                    // Get frequency data
                    const bufferLength = analyserNode.frequencyBinCount;
                    const dataArray = new Uint8Array(bufferLength);
                    analyserNode.getByteFrequencyData(dataArray);

                    // Calculate average power
                    let sumL = 0;
                    let sumR = 0;

                    // Simulate stereo by splitting the frequency range
                    const midPoint = Math.floor(bufferLength / 2);

                    for (let i = 0; i < midPoint; i++) {
                        sumL += dataArray[i];
                    }

                    for (let i = midPoint; i < bufferLength; i++) {
                        sumR += dataArray[i];
                    }

                    const avgL = sumL / midPoint;
                    const avgR = sumR / (bufferLength - midPoint);

                    // Get current volume for sensitivity adjustment
                    const volumeSlider = document.getElementById('volumeSlider');
                    const volumeLevel = volumeSlider ? volumeSlider.value / 100 : 1.0;

                    // Apply volume compensation factor
                    let compensationFactor = 1.0;
                    if (volumeLevel < 0.8) {
                        // Calculate compensation factor (stronger at lower volumes)
                        compensationFactor = Math.pow(1 / Math.max(0.1, volumeLevel), 0.5);
                        compensationFactor = Math.min(compensationFactor, 3.0);
                    }

                    // Apply compensation
                    const adjustedAvgL = avgL * compensationFactor;
                    const adjustedAvgR = avgR * compensationFactor;

                    // Convert to dB scale (approximate)
                    // 0-255 -> -60-0 dB (standard audio range)
                    const dbL = ((adjustedAvgL / 255) * 60) - 60;
                    const dbR = ((adjustedAvgR / 255) * 60) - 60;

                    // Update VU meter
                    updateVUMeter(dbL, dbR);
                } catch (e) {
                    logDebug(`Error updating VU meter: ${e.message}`, 'error');
                }
            }, 50);
        }
    }

    // Simpler VU meter animation as fallback
    function startSimpleVUMeterAnimation() {
        logDebug('Starting simpler VU meter animation as fallback', 'info');

        vuMeterInterval = setInterval(function() {
            if (!isPlaying) return;

            // Get current volume level for sensitivity adjustment
            const volumeSlider = document.getElementById('volumeSlider');
            const volumeLevel = volumeSlider ? volumeSlider.value / 100 : 1.0;

            // Calculate a more realistic audio pattern with dynamic range
            // Base level is influenced by volume - lower volumes still show some activity
            const minLevel = 0.1 + (volumeLevel * 0.3);
            const baseLevel = minLevel + (Math.random() * 0.3);

            // Occasional peaks for realism
            const hasPeak = Math.random() > 0.8;
            const peakMultiplier = hasPeak ? Math.random() * 0.5 + 0.5 : 0;

            // Apply volume-based compensation to make meters more responsive at low volumes
            let compensationFactor = 1.0;
            if (volumeLevel < 0.8) {
                compensationFactor = Math.pow(1 / Math.max(0.1, volumeLevel), 0.5);
                compensationFactor = Math.min(compensationFactor, 3.0);
            }

            // Calculate level with compensation
            const effectiveLevel = baseLevel * compensationFactor;

            // Typical broadcast audio stays around -18 to -12 dB with peaks to -6 dB
            const dbL = -50 + (effectiveLevel * 35) + (peakMultiplier * 20);
            const dbR = -50 + (effectiveLevel * 35) + (peakMultiplier * 20 * Math.random());

            // Update VU meter
            updateVUMeter(dbL, dbR);
        }, 80);
    }

    // Update VU meter display based on audio levels in dB
    function updateVUMeter(dbL, dbR) {
        // Get VU meter elements
        const vuMeterLeftValue = document.getElementById('vuMeterLeftValue');
        const vuMeterRightValue = document.getElementById('vuMeterRightValue');
        const vuMeterLeftPeak = document.getElementById('vuMeterLeftPeak');
        const vuMeterRightPeak = document.getElementById('vuMeterRightPeak');
        const rmsValueDisplay = document.querySelector('.vu-rms-value');
        const peakValueDisplay = document.querySelector('.vu-peak-value');

        // Standard VU meter range is -30dB to 0dB
        // Limit range for display purposes
        const minDB = -60;
        const maxDB = 0;
        const vuMeterMinDB = -30;
        const vuMeterMaxDB = 0;

        // Clamp values to range
        const clampedDbL = Math.max(Math.min(dbL, maxDB), minDB);
        const clampedDbR = Math.max(Math.min(dbR, maxDB), minDB);

        // Calculate meter positions (0-100%)
        // Map the dB range to percentage for the meter
        // For VU meter: -30dB = 0%, 0dB = 100%
        // For number display: full range
        const leftPercent = ((clampedDbL - vuMeterMinDB) / (vuMeterMaxDB - vuMeterMinDB)) * 100;
        const rightPercent = ((clampedDbR - vuMeterMinDB) / (vuMeterMaxDB - vuMeterMinDB)) * 100;

        // Clamp percentage to 0-100 range
        const clampedLeftPercent = Math.max(Math.min(leftPercent, 100), 0);
        const clampedRightPercent = Math.max(Math.min(rightPercent, 100), 0);

        // Update meter values
        if (vuMeterLeftValue) vuMeterLeftValue.style.width = `${clampedLeftPercent}%`;
        if (vuMeterRightValue) vuMeterRightValue.style.width = `${clampedRightPercent}%`;

        // Update numerical displays with 1 decimal place
        if (rmsValueDisplay) {
            if (clampedDbL <= -60) {
                rmsValueDisplay.textContent = '-∞ dB';
            } else {
                rmsValueDisplay.textContent = `${clampedDbL.toFixed(1)} dB`;
            }
        }

        // Update peak values and display
        // Track the highest level
        if (clampedDbL > peakLeft) {
            peakLeft = clampedDbL;

            // Reset peak hold timeout
            if (peakHoldTimeoutLeft) clearTimeout(peakHoldTimeoutLeft);

            // Set timeout to decay peak value
            peakHoldTimeoutLeft = setTimeout(() => {
                peakLeft = -100;
                if (vuMeterLeftPeak) vuMeterLeftPeak.style.left = '0%';
            }, peakHoldTime);

            // Update peak value display
            if (peakValueDisplay) {
                if (peakLeft <= -60) {
                    peakValueDisplay.textContent = '-∞ dB';
                } else {
                    peakValueDisplay.textContent = `${peakLeft.toFixed(1)} dB`;
                }
            }
        }

        if (clampedDbR > peakRight) {
            peakRight = clampedDbR;

            // Reset peak hold timeout
            if (peakHoldTimeoutRight) clearTimeout(peakHoldTimeoutRight);

            // Set timeout to decay peak value
            peakHoldTimeoutRight = setTimeout(() => {
                peakRight = -100;
                if (vuMeterRightPeak) vuMeterRightPeak.style.left = '0%';
            }, peakHoldTime);
        }

        // Update peak indicators
        const peakLeftPercent = ((peakLeft - vuMeterMinDB) / (vuMeterMaxDB - vuMeterMinDB)) * 100;
        const peakRightPercent = ((peakRight - vuMeterMinDB) / (vuMeterMaxDB - vuMeterMinDB)) * 100;

        // Clamp percentage to 0-100 range
        const clampedPeakLeftPercent = Math.max(Math.min(peakLeftPercent, 100), 0);
        const clampedPeakRightPercent = Math.max(Math.min(peakRightPercent, 100), 0);

        if (vuMeterLeftPeak) vuMeterLeftPeak.style.left = `${clampedPeakLeftPercent}%`;
        if (vuMeterRightPeak) vuMeterRightPeak.style.left = `${clampedPeakRightPercent}%`;
    }

    // Stop VU meter animation
    function stopVUMeterAnimation() {
        if (vuMeterInterval) {
            clearInterval(vuMeterInterval);
            vuMeterInterval = null;
        }

        // Reset VU meters
        const vuMeterLeftValue = document.getElementById('vuMeterLeftValue');
        const vuMeterRightValue = document.getElementById('vuMeterRightValue');
        const vuMeterLeftPeak = document.getElementById('vuMeterLeftPeak');
        const vuMeterRightPeak = document.getElementById('vuMeterRightPeak');
        const rmsValueDisplay = document.querySelector('.vu-rms-value');
        const peakValueDisplay = document.querySelector('.vu-peak-value');

        if (vuMeterLeftValue) vuMeterLeftValue.style.width = '0%';
        if (vuMeterRightValue) vuMeterRightValue.style.width = '0%';
        if (vuMeterLeftPeak) vuMeterLeftPeak.style.left = '0%';
        if (vuMeterRightPeak) vuMeterRightPeak.style.left = '0%';
        if (rmsValueDisplay) rmsValueDisplay.textContent = '-∞ dB';
        if (peakValueDisplay) peakValueDisplay.textContent = '-∞ dB';

        // Reset peak values
        peakLeft = -100;
        peakRight = -100;

        // Clear peak hold timeouts
        if (peakHoldTimeoutLeft) {
            clearTimeout(peakHoldTimeoutLeft);
            peakHoldTimeoutLeft = null;
        }

        if (peakHoldTimeoutRight) {
            clearTimeout(peakHoldTimeoutRight);
            peakHoldTimeoutRight = null;
        }
    }

    // Clean up audio analysis resources
    function cleanupAudioAnalysis() {
        logDebug('Cleaning up audio analysis resources', 'info');

        // Stop script processor to prevent memory leaks
        if (scriptProcessorNode) {
            try {
                scriptProcessorNode.disconnect();
                scriptProcessorNode.onaudioprocess = null;
                scriptProcessorNode = null;
                logDebug('Script processor node disconnected', 'info');
            } catch (e) {
                logDebug(`Error disconnecting script processor: ${e.message}`, 'error');
            }
        }

        // Disconnect and clean up analyzer
        if (analyserNode) {
            try {
                analyserNode.disconnect();
                analyserNode = null;
                logDebug('Analyzer node disconnected', 'info');
            } catch (e) {
                logDebug(`Error disconnecting analyzer: ${e.message}`, 'error');
            }
        }

        // Disconnect and clean up gain node
        if (gainNode) {
            try {
                gainNode.disconnect();
                gainNode = null;
                logDebug('Gain node disconnected', 'info');
            } catch (e) {
                logDebug(`Error disconnecting gain node: ${e.message}`, 'error');
            }
        }

        // Disconnect source node
        if (audioSourceNode) {
            try {
                audioSourceNode.disconnect();
                audioSourceNode = null;
                logDebug('Audio source node disconnected', 'info');
            } catch (e) {
                logDebug(`Error disconnecting audio source: ${e.message}`, 'error');
            }
        }

        // Close audio context
        if (audioContext) {
            try {
                audioContext.close().then(() => {
                    logDebug('Audio context closed', 'info');
                    audioContext = null;
                }).catch(e => {
                    logDebug(`Error closing audio context: ${e.message}`, 'error');
                    audioContext = null;
                });
            } catch (e) {
                logDebug(`Error closing audio context: ${e.message}`, 'error');
                audioContext = null;
            }
        }
    }

    // Adjust volume setting
    function adjustVolume() {
        const volumeSlider = document.getElementById('volumeSlider');
        const volumeLevel = document.getElementById('volumeLevel');

        if (!volumeSlider || !volumeLevel) return;

        const volume = volumeSlider.value;

        // Update volume display
        volumeLevel.textContent = volume + '%';

        // Apply volume to current audio element
        if (currentAudio) {
            currentAudio.volume = volume / 100;
            logDebug(`Volume set to ${volume}%`, 'info');
        }

        // Also update gain node if available for consistent volume across audio APIs
        if (gainNode) {
            gainNode.gain.value = volume / 100;
        }

        // Update icon based on volume level
        const volumeIcon = document.querySelector('.volume-icon');
        if (volumeIcon) {
            if (volume == 0) {
                volumeIcon.textContent = '🔈';
            } else if (volume < 50) {
                volumeIcon.textContent = '🔉';
            } else {
                volumeIcon.textContent = '🔊';
            }
        }
    }

    // Update On Air indicator
    function updateOnAirIndicator() {
        const onAirIndicator = document.getElementById('onAirIndicator');
        if (!onAirIndicator) return;

        if (isPlaying && currentIndex !== null && mounts && mounts.length > currentIndex) {
            const mount = mounts[currentIndex];
            if (mount && mount.online) {
                onAirIndicator.textContent = mount.is_autodj ? 'AUTO-DJ' : 'ON AIR';
                onAirIndicator.className = mount.is_autodj ? 'on-air' : 'on-air live';
            } else {
                onAirIndicator.textContent = 'OFFLINE';
                onAirIndicator.className = 'on-air';
            }
        } else {
            onAirIndicator.textContent = 'OFFLINE';
            onAirIndicator.className = 'on-air';
        }
    }

    // Update listener statistics
    function updateListenerStats() {
        if (!mounts || mounts.length === 0 || currentIndex === null) return;

        const mount = mounts[currentIndex];
        if (!mount) return;

        const currentListeners = document.getElementById('currentListeners');
        const peakListeners = document.getElementById('peakListeners');
        const bitrate = document.getElementById('bitrate');

        if (currentListeners) currentListeners.textContent = mount.listeners;
        if (peakListeners) peakListeners.textContent = mount.listener_peak;
        if (bitrate) bitrate.textContent = mount.bitrate + ' kbps';
    }

    // Get current mount bitrate
    function getCurrentMountBitrate() {
        if (mounts && mounts.length > currentIndex) {
            return mounts[currentIndex].bitrate;
        }
        return "unknown";
    }

    // Show/hide buffering indicators
    function showBuffering(isBuffering) {
        const overlay = document.getElementById('bufferOverlay');
        const indicator = document.getElementById('bufferingIndicator');

        if (!overlay || !indicator) return;

        // Clear any existing timeout
        if (bufferTimeout) {
            clearTimeout(bufferTimeout);
            bufferTimeout = null;
        }

        if (isBuffering) {
            const bitrate = getCurrentMountBitrate();
            const bufferDetails = document.getElementById('bufferDetails');
            if (bufferDetails) {
                bufferDetails.textContent = "Loading " + bitrate + " kbps stream... please wait";
            }

            overlay.style.display = 'flex';
            indicator.style.display = 'inline';

            // Maximum buffer display time
            bufferTimeout = setTimeout(function() {
                overlay.style.display = 'none';
            }, 15000);
        } else {
            setTimeout(function() {
                overlay.style.display = 'none';
                indicator.style.display = 'none';
            }, 500);
        }
    }

    // Clean up audio resources
    function cleanupAudio() {
        logDebug('Cleaning up audio resources', 'info');

        // Stop playback
        if (currentAudio) {
            try {
                currentAudio.pause();
                currentAudio.removeAttribute('src');
                currentAudio.load();
            } catch (e) {
                logDebug(`Error cleaning up audio element: ${e.message}`, 'error');
            }
        }

        // Stop VU meter animation
        stopVUMeterAnimation();

        // Clean up audio analysis system
        cleanupAudioAnalysis();

        // Reset playback state
        isPlaying = false;
        showBuffering(false);
    }

    // Switch to a different mount point
    function switchMount() {
        const mountSelector = document.getElementById('mountSelector');
        if (!mountSelector) return;

        logDebug(`Switching to mount index ${mountSelector.value}`, 'info');

        // Clean up current audio resources
        cleanupAudio();

        // Update current index
        currentIndex = parseInt(mountSelector.value);

        // Hide all player blocks
        document.querySelectorAll('[id^="playerBlock"]').forEach(b => b.style.display = 'none');

        // Show selected block
        currentBlock = document.getElementById('playerBlock' + currentIndex);
        if (currentBlock) {
            currentBlock.style.display = 'block';
        }

        // Update audio reference
        currentAudio = document.getElementById('audio' + currentIndex);
        if (currentAudio) {
            setupAudioEvents(currentAudio);

            // Apply current volume setting
            const volumeSlider = document.getElementById('volumeSlider');
            if (volumeSlider && currentAudio) {
                currentAudio.volume = volumeSlider.value / 100;
            }
        }

        // Update displays
        pollNowPlaying();
        updateNowPlaying();
        updatePlayingStatus();
        updateOnAirIndicator();
        updateListenerStats();
        updateStationDetails();
        updateDocumentTitle();
    }

    // Update currently playing track info
    function updateNowPlaying() {
        if (!currentBlock) return;

        const titleElement = currentBlock.querySelector('.nowplaying');
        const currentTitle = document.getElementById('currentTitle');

        if (titleElement && currentTitle) {
            currentTitle.textContent = titleElement.textContent;
        }
    }

    // Update playing status indicator
    function updatePlayingStatus() {
        const indicator = document.getElementById('playingIndicator');
        if (!indicator) return;

        if (currentAudio && !currentAudio.paused) {
            indicator.textContent = "▶️ NOW PLAYING";
        } else {
            indicator.textContent = "⏸️ STOPPED";
        }
    }

    // Play audio stream
    function playAudio() {
        logDebug(`[playAudio] Play initiated`);
        if (!currentAudio) {
            logDebug('No audio element found', 'error');
            return;
        }

        logDebug('Starting audio playback', 'info');

        try {
            // Clean up first for a fresh start
            cleanupAudio();

            // Re-get the audio element
            currentAudio = document.getElementById('audio' + currentIndex);
            if (!currentAudio) {
                logDebug('Cannot find audio element after cleanup', 'error');
                return;
            }

            // Set up events again
            setupAudioEvents(currentAudio);

            // Apply volume setting
            const volumeSlider = document.getElementById('volumeSlider');
            if (volumeSlider && currentAudio) {
                currentAudio.volume = volumeSlider.value / 100;
            }

            // Begin buffering indicators
            showBuffering(true);

            // Force loading audio
            currentAudio.load();

            // Add small delay to help buffering
            setTimeout(() => {
                logDebug('Attempting to play audio', 'info');

                // Use promise-based play for proper error handling
                currentAudio.play()
                    .then(() => {
                        logDebug('Audio playback started successfully', 'info');
                        isPlaying = true;
                        updatePlayingStatus();
                        updateOnAirIndicator();
                        updateDocumentTitle();
                    })
                    .catch(e => {
                        logDebug(`Audio play failed: ${e.message}`, 'error');
                        showBuffering(false);
                        alert(`Unable to play this stream: ${e.message}`);
                    });
            }, 300);
        } catch (e) {
            logDebug(`Play error: ${e}`, 'error');
            showBuffering(false);
        }
    }

    // Pause audio playback
    function pauseAudio() {
        if (!currentAudio) return;

        logDebug('Pausing audio', 'info');
        currentAudio.pause();
        showBuffering(false);
    }

    // Stop audio playback completely
    function stopAudio() {
        logDebug('Stopping audio', 'info');
        cleanupAudio();

        // Restore audio element reference
        currentAudio = document.getElementById('audio' + currentIndex);
        if (currentAudio) {
            setupAudioEvents(currentAudio);
        }

        updatePlayingStatus();
        updateOnAirIndicator();
    }

    // Reload the page
    function reloadPage() {
        // Save settings
        localStorage.setItem('selectedMount', currentIndex);

        const volumeSlider = document.getElementById('volumeSlider');
        if (volumeSlider) {
            localStorage.setItem('volume', volumeSlider.value);
        }

        // Save song history
        if (songHistory && songHistory.length > 0) {
            try {
                localStorage.setItem('songHistory', JSON.stringify(songHistory));
            } catch (e) {
                logDebug(`Error saving song history: ${e.message}`, 'error');
            }
        }

        // Reload with debug parameter if in debug mode
        const debugParam = debugMode ? '&debug=true' : '';
        window.location.href = window.location.pathname + '?url=' + reloadUrl + debugParam + '&t=' + Date.now();
    }

    // Restore saved settings
    (function() {
        // Restore mount selection
        const savedMount = localStorage.getItem('selectedMount');
        if (savedMount !== null) {
            const mountSelector = document.getElementById('mountSelector');
            if (mountSelector && mountSelector.options.length > parseInt(savedMount)) {
                mountSelector.value = savedMount;
                switchMount();
            }
        }

        // Restore volume setting
        const savedVolume = localStorage.getItem('volume');
        if (savedVolume !== null) {
            const volumeSlider = document.getElementById('volumeSlider');
            if (volumeSlider) {
                volumeSlider.value = savedVolume;
                adjustVolume();
            }
        }

        // Restore song history
        try {
            const savedHistory = localStorage.getItem('songHistory');
            if (savedHistory) {
                songHistory = JSON.parse(savedHistory);
                // Update display
                updateSongHistoryDisplay();
            }
        } catch (e) {
            logDebug(`Error restoring song history: ${e.message}`, 'error');
        }
    })();

    // Set up metadata polling for streams that don't trigger events
    // This ensures we catch title changes even on streams that don't update metadata properly
    setInterval(function() {
        if (isPlaying && currentAudio) {
            // Check if we can fetch metadata from the stream
            try {
                // Some streams update their nowplaying info via XHR without triggering events
                // We'll check the DOM element where the current title should be displayed
                const mountBlock = document.getElementById('playerBlock' + currentIndex);
                if (mountBlock) {
                    const nowPlayingElement = mountBlock.querySelector('.nowplaying');
                    if (nowPlayingElement) {
                        const currentTitleElement = document.getElementById('currentTitle');
                        if (currentTitleElement && currentTitleElement.textContent !== nowPlayingElement.textContent) {
                            // Update the displayed title
                            currentTitleElement.textContent = nowPlayingElement.textContent;

                            // Update document title
                            updateDocumentTitle();

                            // Add to history if it's changed
                            if (nowPlayingElement.textContent !== currentSongTitle) {
                                addSongToHistory(nowPlayingElement.textContent);
                            }
                        }
                    }
                }
            } catch (e) {
                logDebug(`Error checking metadata: ${e.message}`, 'error');
            }
        }
    }, 10000); // Check every 10 seconds

    // Handle iOS audio unblocking
    document.addEventListener('touchstart', function() {
        // On iOS, touch interaction is needed to unblock audio
        if (currentAudio && currentAudio.paused) {
            logDebug('Touch interaction detected - preparing audio for iOS', 'info');

            // Load the audio
            currentAudio.load();

            // Create and resume a temporary audio context to unblock WebAudio API
            try {
                const tempContext = new (window.AudioContext || window.webkitAudioContext)();
                tempContext.resume().then(() => {
                    logDebug('Audio context unblocked via touch', 'info');
                }).catch(e => {
                    logDebug(`Failed to unblock audio: ${e.message}`, 'error');
                });
            } catch (e) {
                logDebug(`Unable to create temporary audio context: ${e.message}`, 'error');
            }
        }
    }, false);

    // Periodically update server uptime display
    setInterval(updateServerUptime, 1000);

    // Ping the stream periodically to keep it alive
    // This helps with some servers that disconnect idle clients
    setInterval(function() {
        if (isPlaying && currentAudio) {
            // Small volume change to refresh the connection
            const currentVolume = currentAudio.volume;
            currentAudio.volume = Math.max(0.01, currentVolume - 0.01);
            setTimeout(() => {
                currentAudio.volume = currentVolume;
            }, 100);

            logDebug('Stream ping to prevent timeout', 'debug');
        }
    }, 30000); // Every 30 seconds
// Ensure toggleAboutModal is defined in the main script block if not already present
(function() {
    if (typeof toggleAboutModal !== "function") {
        window.toggleAboutModal = function() {
            const modal = document.getElementById('aboutModal');
            if (!modal) {
                console.error('❌ About modal element not found');
                return;
            }
            const currentDisplay = window.getComputedStyle(modal).display;
            modal.style.display = (currentDisplay === 'none') ? 'block' : 'none';
        };
    }
})();

    function pollNowPlaying() {
        if (!mounts || !mounts.length) return;
        const mount = mounts[currentIndex];
        if (!mount || !mount.listenurl) return;

        let statusUrl = mount.listenurl.replace(/\/[^\/]+$/, '/status-json.xsl');

        fetch(statusUrl, { cache: "no-store" })
            .then(response => response.json())
            .then(data => {
                let source = data.icestats?.source;
                if (!source) return;

                if (!Array.isArray(source)) source = [source];
                const matched = source.find(s => s.listenurl === mount.listenurl);

                const title = matched?.yp_currently_playing || matched?.title || 'No Title Streaming';
                const nowPlayingElem = document.getElementById('currentTitle');

                if (nowPlayingElem) {
                    nowPlayingElem.textContent = title;
                    nowPlayingElem.setAttribute('data-title', title);
                }

                document.title = `🎵 ${title} | ${initialStationName}`;
            })
            .catch(console.error);
    }

    // Start polling every 5 seconds
    setInterval(pollNowPlaying, 5000);
</script>
</body>
</html>