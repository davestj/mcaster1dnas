#!/bin/bash
#
# Mcaster1DNAS Control Script
# Start, stop, restart, and check status of Mcaster1DNAS server
#

MCASTER1_BIN="/var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1"
CONFIG_FILE="/var/www/mcaster1.com/mcaster1dnas/mcaster1-production.xml"
PID_FILE="/var/www/mcaster1.com/mcaster1dnas/build/mcaster1.pid"
LOG_DIR="/var/www/mcaster1.com/mcaster1dnas/build/logs"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[✓]${NC} $1"
}

print_error() {
    echo -e "${RED}[✗]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

check_running() {
    if [ -f "$PID_FILE" ]; then
        PID=$(cat "$PID_FILE")
        if ps -p "$PID" > /dev/null 2>&1; then
            return 0  # Running
        else
            rm -f "$PID_FILE"  # Stale PID file
            return 1  # Not running
        fi
    fi
    return 1  # Not running
}

start() {
    echo "Starting Mcaster1DNAS..."

    if check_running; then
        print_warning "Mcaster1DNAS is already running (PID: $(cat $PID_FILE))"
        return 1
    fi

    if [ ! -f "$MCASTER1_BIN" ]; then
        print_error "Binary not found at $MCASTER1_BIN"
        print_error "Run './build.sh' first to compile"
        return 1
    fi

    if [ ! -f "$CONFIG_FILE" ]; then
        print_error "Config file not found at $CONFIG_FILE"
        return 1
    fi

    # Start the server
    cd /var/www/mcaster1.com/mcaster1dnas
    "$MCASTER1_BIN" -c "$CONFIG_FILE" -b

    sleep 2

    if check_running; then
        print_status "Mcaster1DNAS started successfully (PID: $(cat $PID_FILE))"
        print_status "HTTP:  http://15.204.91.208:9330/"
        print_status "HTTPS: https://15.204.91.208:9443/"
        print_status "Admin: https://15.204.91.208:9443/admin/"
        return 0
    else
        print_error "Failed to start Mcaster1DNAS"
        print_error "Check logs: tail -n 50 $LOG_DIR/error.log"
        return 1
    fi
}

stop() {
    echo "Stopping Mcaster1DNAS..."

    if ! check_running; then
        print_warning "Mcaster1DNAS is not running"
        return 1
    fi

    PID=$(cat "$PID_FILE")
    kill "$PID" 2>/dev/null

    # Wait up to 10 seconds for graceful shutdown
    for i in {1..10}; do
        if ! ps -p "$PID" > /dev/null 2>&1; then
            rm -f "$PID_FILE"
            print_status "Mcaster1DNAS stopped successfully"
            return 0
        fi
        sleep 1
    done

    # Force kill if still running
    print_warning "Graceful shutdown failed, forcing..."
    kill -9 "$PID" 2>/dev/null
    rm -f "$PID_FILE"
    print_status "Mcaster1DNAS force stopped"
    return 0
}

restart() {
    echo "Restarting Mcaster1DNAS..."
    stop
    sleep 2
    start
}

status() {
    if check_running; then
        PID=$(cat "$PID_FILE")
        UPTIME=$(ps -p "$PID" -o etime= | tr -d ' ')
        print_status "Mcaster1DNAS is running"
        echo "  PID:    $PID"
        echo "  Uptime: $UPTIME"
        echo "  Config: $CONFIG_FILE"
        echo ""
        echo "Listening on:"
        netstat -tulpn 2>/dev/null | grep "$PID" | grep mcaster1 || ss -tulpn 2>/dev/null | grep "$PID"
        return 0
    else
        print_error "Mcaster1DNAS is not running"
        return 1
    fi
}

logs() {
    if [ ! -d "$LOG_DIR" ]; then
        print_error "Log directory not found: $LOG_DIR"
        return 1
    fi

    echo "Recent error log (last 30 lines):"
    echo "=================================="
    tail -n 30 "$LOG_DIR/error.log" 2>/dev/null || print_error "No error log found"
    echo ""
    echo "To follow logs in real-time:"
    echo "  tail -f $LOG_DIR/error.log"
    echo "  tail -f $LOG_DIR/access.log"
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        restart
        ;;
    status)
        status
        ;;
    logs)
        logs
        ;;
    *)
        echo "Mcaster1DNAS Control Script"
        echo ""
        echo "Usage: $0 {start|stop|restart|status|logs}"
        echo ""
        echo "Commands:"
        echo "  start    - Start Mcaster1DNAS server"
        echo "  stop     - Stop Mcaster1DNAS server"
        echo "  restart  - Restart Mcaster1DNAS server"
        echo "  status   - Show server status"
        echo "  logs     - Display recent error logs"
        echo ""
        exit 1
        ;;
esac

exit $?
