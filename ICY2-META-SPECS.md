
ICY-META v2.1+ Protocol Specification (FULL + Legacy Compatibility & Auth)

Project: mcaster1 DNAS / CasterClub Streaming Protocol Initiative
Version: 2.1+
Maintained By: CasterClub / mcaster1 Core Team
License: Open Spec (CSSI)
🔰 Overview

ICY-META v2.1+ is a comprehensive streaming metadata and control protocol, preserving legacy SHOUTcast compatibility while adding full support for podcasts, social media integration, video broadcast metadata, and stream identity verification.

It powers next-generation platforms such as mcaster1 DNAS and yp.casterclub.com, enabling content creators to stream, post, promote, or schedule metadata-rich content with or without live audio/video feeds.
📡 Legacy ICY 1.x Headers (Preserved for Compatibility)
Header 	Type 	Description
icy-name 	String 	Station or stream display name
icy-genre 	String 	Genre or content type
icy-url 	URL 	Station homepage or site
icy-pub 	Boolean 	Public listing flag (1 = yes)
icy-br 	Integer 	Bitrate in kbps
icy-metaint 	Integer 	Interval in bytes between metadata blocks
🔐 Legacy Authentication (Backwards-Compatible)
Field 	Type 	Description
adminpassword 	String 	Admin interface password (for HTML/admin.cgi)
password 	String 	Source encoder password for streaming
user 	String 	Optional username (used in SHOUTcast v2 or Icecast2)


SOURCE password@hostname:port/stream

⸻

🧩 Core ICY-META v2.1+ Fields

Header  Type    Description
icy-metadata-version    String  Metadata version (set to 2.1)
icy-meta-station-id String  Unique global station ID
icy-meta-certissuer-id  String  Certificate authority ID
icy-meta-cert-rootca    String  Root CA hash or fingerprint
icy-meta-certificate    PEM Base64 PEM cert
icy-meta-verification-status    Enum    unverified, pending, verified, gold

⸻

🎧 Audio + Podcast Metadata

Header  Type    Description
icy-meta-dj-handle  String  Current DJ/host handle
icy-meta-podcast-host   String  Podcast creator/host name
icy-meta-podcast-rss    URL Podcast RSS feed
icy-meta-podcast-episode    String  Episode title or ID
icy-meta-duration   Integer Duration in seconds
icy-meta-language   String  Language tag (e.g., en, es)

⸻

🎥 Video Streaming & Metadata Support

Header  Type    Description
icy-meta-videotype  Enum    live, short, clip, trailer, ad
icy-meta-videolink  URL Link to video content
icy-meta-videotitle String  Title of the video
icy-meta-videoposter    URL Thumbnail or preview image
icy-meta-videochannel   String  Creator/uploader/channel handle
icy-meta-videoplatform  Enum    youtube, tiktok, twitch, etc.
icy-meta-videoduration  Integer Length in seconds
icy-meta-videostart ISO8601 Scheduled start time
icy-meta-videolive  Boolean Is it currently live?
icy-meta-videocodec String  Video codec (e.g., h264, vp9)
icy-meta-videofps   Integer Frames per second
icy-meta-videoresolution    String  e.g., 1080p, 4K, 720x1280
icy-meta-videonsfw  Boolean NSFW indicator

⸻

📱 Social, Discovery, Branding

Header  Type    Description
icy-meta-emoji  Emoji[] Mood or emotion indicators
icy-meta-hashtag-array  String[]    Searchable tags
icy-meta-social-twitter String  Twitter/X handle
icy-meta-social-ig  String  Instagram username
icy-meta-social-tiktok  String  TikTok profile name
icy-meta-social-linktree    URL Unified profile (Linktree, etc.)

⸻

🔐 Access & AI Compliance

Header  Type    Description
icy-auth-token-key  JWT Optional access token
icy-meta-nsfw   Boolean NSFW content flag
icy-meta-ai-generator   Boolean AI-generated content flag
icy-meta-geo-region String  Location (e.g., us-west)

⸻

📦 Sample Scenarios

🎙 Podcast Episode Metadata Push

icy-name: FutureTalks
icy-meta-podcast-host: Sasha Tran
icy-meta-podcast-episode: S4E1 – Decentralized Rights
icy-meta-podcast-rss: https://futuretalks.fm/feed.xml
icy-meta-language: en
icy-meta-verification-status: verified

📸 TikTok Post Push (No Audio Needed)

icy-name: @DropMaster
icy-meta-videotype: short
icy-meta-videolink: https://tiktok.com/@dropmaster/video/7739201
icy-meta-videochannel: @dropmaster
icy-meta-videoplatform: tiktok
icy-meta-videoposter: https://cdn.tiktok.com/posters/7739201.jpg
icy-meta-emoji: 🎵🔥🎥
icy-meta-hashtag-array: ["#beatdrop", "#shorts"]

📺 YouTube Livestream Broadcast

icy-name: ChillZone FM
icy-meta-videotype: live
icy-meta-videolink: https://youtube.com/watch?v=live543
icy-meta-videotitle: Synthwave All Night
icy-meta-videoplatform: youtube
icy-meta-videoresolution: 1080p
icy-meta-videoduration: 7200
icy-meta-videolive: true
icy-meta-verification-status: verified

⸻

🛠 Integration Targets
    •   🎧 DSPs: SAM Broadcaster, Edcast, BUTT, Rocket, Mixxx
    •   🖥 Admin/GUI: mcasterClient, metadata-pusher agents
    •   📡 Directory: yp.casterclub.com via /8.json, /status.html, or /push
    •   🔁 Webhook/event-forwarding for real-time content relay

⸻

📎 Maintainer Contact

CasterClub Streaming Standards Initiative (CSSI)
📧 specs@casterclub.com
🌐 https://casterclub.com/specs/icy-2
💻 GitHub: https://github.com/casterclub/specs
🪪 License: Open Specification / Attribution Preferred


