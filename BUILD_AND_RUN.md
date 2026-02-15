# Mcaster1DNAS Build & Run Instructions

## 🔨 Complete Build Process

### Step 1: Regenerate Build System
```bash
cd /var/www/mcaster1.com/mcaster1dnas

# Clean any autotools cache
rm -rf autom4te.cache config.h config.h.in~ config.status

# Regenerate configure script with updated naming
./autogen.sh
```

**Expected Output:**
```
Generating configure script...
Running aclocal...
Running autoheader...
Running automake...
Running autoconf...
Done! Run ./configure now
```

### Step 2: Configure with All Features
```bash
./configure \
  --prefix=/var/www/mcaster1.com/mcaster1dnas/build \
  --with-openssl \
  --with-ogg \
  --with-vorbis \
  --with-theora \
  --with-speex \
  --with-curl
```

**Expected Output (last few lines):**
```
Features:
  XML     : yes
  XSLT    : yes
  SSL     : yes
  CURL    : yes
  Ogg     : yes
  Vorbis  : yes
  Theora  : yes
  Speex   : yes

Type "make" to compile Mcaster1DNAS
```

**Troubleshooting:**
If SSL not found:
```bash
# On Debian/Ubuntu
sudo apt-get install libssl-dev

# On CentOS/RHEL
sudo yum install openssl-devel
```

### Step 3: Compile
```bash
# Clean previous builds
make clean

# Compile (use all CPU cores for faster build)
make -j$(nproc)
```

**Expected Output (final line):**
```
make[1]: Leaving directory '/var/www/mcaster1.com/mcaster1dnas/src'
```

### Step 4: Install to Build Directory
```bash
make install
```

**Expected Output:**
```
Making install in src
...
Configuration examples are now in /var/www/mcaster1.com/mcaster1dnas/build/share/doc/mcaster1dnas
```

### Step 5: Verify Installation
```bash
# Check the binary was created
ls -lh /var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1

# Check version
/var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1 --version
```

**Expected Output:**
```
Mcaster1DNAS 2.5.0
```

## 📁 Directory Setup

### Create Log Directory
```bash
mkdir -p /var/www/mcaster1.com/mcaster1dnas/build/logs
chmod 755 /var/www/mcaster1.com/mcaster1dnas/build/logs
```

### Verify SSL Certificate
```bash
# Check SSL cert exists and has correct permissions
ls -l /var/www/mcaster1.com/mcaster1dnas/ssl/mcaster1dnas.pem

# Verify certificate is valid
openssl x509 -in /var/www/mcaster1.com/mcaster1dnas/ssl/mcaster1dnas.pem -noout -text | head -20
```

## ⚙️ Configuration

### Production Configuration File
**Location:** `/var/www/mcaster1.com/mcaster1dnas/mcaster1-production.xml`

**Key Settings:**
- **HTTP Port:** 9330 on 15.204.91.208
- **HTTPS Port:** 9443 on 15.204.91.208 (SSL enabled)
- **SSL Certificate:** `/var/www/mcaster1.com/mcaster1dnas/ssl/mcaster1dnas.pem`
- **Web Root:** `/var/www/mcaster1.com/mcaster1dnas/web`
- **Admin Root:** `/var/www/mcaster1.com/mcaster1dnas/admin`
- **Logs:** `/var/www/mcaster1.com/mcaster1dnas/build/logs/`

### ⚠️ Update Default Passwords
**CRITICAL:** Before running in production, edit the config and change:
```bash
nano /var/www/mcaster1.com/mcaster1dnas/mcaster1-production.xml
```

Update these lines:
```xml
<source-password>YOUR_SECURE_PASSWORD_HERE</source-password>
<relay-password>YOUR_SECURE_PASSWORD_HERE</relay-password>
<admin-password>YOUR_SECURE_ADMIN_PASSWORD</admin-password>
```

## 🚀 Running Mcaster1DNAS

### Test Run (Foreground Mode)
```bash
cd /var/www/mcaster1.com/mcaster1dnas

# Run in foreground to see output
/var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1 \
  -c mcaster1-production.xml
```

**Expected Output:**
```
INFO main/main Mcaster1DNAS server started
INFO connection/connection_setup_sockets_ssl SSL certificate found at /var/www/mcaster1.com/mcaster1dnas/ssl/mcaster1dnas.pem
INFO connection/connection_listen_sockets listening on port 9330
INFO connection/connection_listen_sockets listening on port 9443 (SSL)
```

**Test the Server:**
Open in browser:
- **HTTP:** http://15.204.91.208:9330/
- **HTTPS:** https://15.204.91.208:9443/
- **Admin:** https://15.204.91.208:9443/admin/stats.xsl

**Stop Test Server:**
Press `Ctrl+C`

### Production Run (Background Daemon)
```bash
cd /var/www/mcaster1.com/mcaster1dnas

# Start as background daemon
/var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1 \
  -c mcaster1-production.xml \
  -b

# Check it's running
ps aux | grep mcaster1

# Check PID file
cat /var/www/mcaster1.com/mcaster1dnas/build/mcaster1.pid
```

### Check Logs
```bash
# Watch error log in real-time
tail -f /var/www/mcaster1.com/mcaster1dnas/build/logs/error.log

# Watch access log in real-time
tail -f /var/www/mcaster1.com/mcaster1dnas/build/logs/access.log

# View last 50 lines of error log
tail -n 50 /var/www/mcaster1.com/mcaster1dnas/build/logs/error.log
```

## 🛑 Stopping the Server

### Method 1: Using PID File
```bash
# Read PID and kill gracefully
kill $(cat /var/www/mcaster1.com/mcaster1dnas/build/mcaster1.pid)
```

### Method 2: Find and Kill Process
```bash
# Find process ID
ps aux | grep mcaster1

# Kill by process ID (replace XXXX with actual PID)
kill XXXX

# Force kill if needed (not recommended)
kill -9 XXXX
```

### Method 3: Killall
```bash
# Kill all mcaster1 processes
killall mcaster1
```

## 🔄 Restart Server

```bash
# Stop the server
kill $(cat /var/www/mcaster1.com/mcaster1dnas/build/mcaster1.pid 2>/dev/null) 2>/dev/null || killall mcaster1

# Wait a moment
sleep 2

# Start again
cd /var/www/mcaster1.com/mcaster1dnas
/var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1 \
  -c mcaster1-production.xml \
  -b

# Verify it started
tail -n 20 /var/www/mcaster1.com/mcaster1dnas/build/logs/error.log
```

## 🔥 Firewall Configuration

### Check Current Firewall Rules
```bash
# UFW (Ubuntu/Debian)
sudo ufw status

# Firewalld (CentOS/RHEL)
sudo firewall-cmd --list-all

# iptables
sudo iptables -L -n | grep 9330
sudo iptables -L -n | grep 9443
```

### Open Required Ports (if needed)
```bash
# UFW
sudo ufw allow 9330/tcp comment 'Mcaster1DNAS HTTP'
sudo ufw allow 9443/tcp comment 'Mcaster1DNAS HTTPS'

# Firewalld
sudo firewall-cmd --permanent --add-port=9330/tcp
sudo firewall-cmd --permanent --add-port=9443/tcp
sudo firewall-cmd --reload

# iptables
sudo iptables -A INPUT -p tcp --dport 9330 -j ACCEPT
sudo iptables -A INPUT -p tcp --dport 9443 -j ACCEPT
sudo service iptables save
```

## 📊 Testing Checklist

### 1. Basic Connectivity
- [ ] HTTP access: http://15.204.91.208:9330/
- [ ] HTTPS access: https://15.204.91.208:9443/
- [ ] Status page loads with modern theme
- [ ] FontAwesome icons display

### 2. Admin Interface
- [ ] Admin login: https://15.204.91.208:9443/admin/
- [ ] Statistics page: https://15.204.91.208:9443/admin/stats.xsl
- [ ] Modern CSS and icons load
- [ ] All navigation links work

### 3. Streaming Test
```bash
# Test with a simple MP3 stream (if you have one)
# Example using ffmpeg to generate a test stream:
ffmpeg -re -f lavfi -i sine=frequency=1000:duration=3600 \
  -acodec libmp3lame -ab 128k -f mp3 \
  icecast://source:hackme@15.204.91.208:9330/test.mp3
```

### 4. Check Server Stats
- [ ] Listeners count updates
- [ ] Source connections shown
- [ ] Bandwidth stats display
- [ ] Log entries appear in access.log

## 🐛 Troubleshooting

### Server Won't Start
```bash
# Check config syntax
/var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1 \
  -c mcaster1-production.xml

# Check error log
tail -n 50 /var/www/mcaster1.com/mcaster1dnas/build/logs/error.log

# Check if port is already in use
netstat -tulpn | grep 9330
netstat -tulpn | grep 9443

# Kill any process using the port
sudo fuser -k 9330/tcp
sudo fuser -k 9443/tcp
```

### SSL Certificate Issues
```bash
# Verify certificate format
openssl x509 -in /var/www/mcaster1.com/mcaster1dnas/ssl/mcaster1dnas.pem -noout -text

# Check if cert includes private key
grep -c "PRIVATE KEY" /var/www/mcaster1.com/mcaster1dnas/ssl/mcaster1dnas.pem
# Should output: 1

# Check permissions
ls -l /var/www/mcaster1.com/mcaster1dnas/ssl/mcaster1dnas.pem
# Should be: -rw------- (600)
```

### CSS/Theme Not Loading
```bash
# Verify CSS files exist
ls -l /var/www/mcaster1.com/mcaster1dnas/admin/mcaster1-modern.css
ls -l /var/www/mcaster1.com/mcaster1dnas/web/mcaster1-modern.css

# Check webroot path in config
grep webroot mcaster1-production.xml

# Check file permissions
chmod 644 /var/www/mcaster1.com/mcaster1dnas/admin/*.css
chmod 644 /var/www/mcaster1.com/mcaster1dnas/web/*.css
```

### Can't Access from Browser
```bash
# Test local connectivity
curl http://localhost:9330/

# Test from server
curl http://15.204.91.208:9330/

# Check if server is listening on correct IP
netstat -tulpn | grep mcaster1

# Should show:
# tcp  0  0  15.204.91.208:9330  0.0.0.0:*  LISTEN  PID/mcaster1
# tcp  0  0  15.204.91.208:9443  0.0.0.0:*  LISTEN  PID/mcaster1
```

## 📝 Quick Command Reference

```bash
# Build from scratch
cd /var/www/mcaster1.com/mcaster1dnas && ./autogen.sh && ./configure --prefix=/var/www/mcaster1.com/mcaster1dnas/build --with-openssl --with-ogg --with-vorbis --with-theora --with-speex --with-curl && make clean && make -j$(nproc) && make install

# Start server
/var/www/mcaster1.com/mcaster1dnas/build/bin/mcaster1 -c /var/www/mcaster1.com/mcaster1dnas/mcaster1-production.xml -b

# Stop server
kill $(cat /var/www/mcaster1.com/mcaster1dnas/build/mcaster1.pid)

# View logs
tail -f /var/www/mcaster1.com/mcaster1dnas/build/logs/error.log

# Check status
ps aux | grep mcaster1
```

## 🎯 Next Steps

1. **Change default passwords** in mcaster1-production.xml
2. **Test all admin functions** via web interface
3. **Configure your broadcasting software** to connect:
   - Server: 15.204.91.208
   - Port: 9330 (HTTP) or 9443 (HTTPS)
   - Password: (your source-password from config)
   - Mount: /stream.mp3 (or your chosen mount point)
4. **Set up systemd service** for auto-start on boot (optional)
5. **Configure monitoring** and log rotation

---

**Build Date:** February 14, 2026
**Version:** Mcaster1DNAS 2.5.0
**Configuration:** Production with SSL on 15.204.91.208:9330/9443
