/**
 * Mcaster1DNAS Utility Functions
 * Live Clock and Page Load Time Tracking
 */

// Page Load Time Tracking
const pageLoadStart = performance.now();

window.addEventListener('load', function() {
    const loadTime = ((performance.now() - pageLoadStart) / 1000).toFixed(3);
    const loadTimeElement = document.getElementById('page-load-time');
    if (loadTimeElement) {
        loadTimeElement.innerHTML = '<i class="fas fa-bolt"></i> Page loaded in ' + loadTime + ' seconds';
    }
});

// Live Clock Update Function
function updateClock() {
    const now = new Date();

    // Time formatting (12-hour with AM/PM)
    let hours = now.getHours();
    const minutes = now.getMinutes().toString().padStart(2, '0');
    const seconds = now.getSeconds().toString().padStart(2, '0');
    const ampm = hours >= 12 ? 'PM' : 'AM';
    hours = hours % 12;
    hours = hours ? hours : 12; // Convert 0 to 12
    const timeString = hours + ':' + minutes + ':' + seconds + ' ' + ampm;

    // Date formatting (Day, Month Date, Year)
    const days = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    const months = ['January', 'February', 'March', 'April', 'May', 'June', 'July', 'August', 'September', 'October', 'November', 'December'];

    const dayName = days[now.getDay()];
    const monthName = months[now.getMonth()];
    const date = now.getDate();
    const year = now.getFullYear();

    const dateString = dayName + ', ' + monthName + ' ' + date + ', ' + year;

    // Update DOM elements
    const timeElement = document.getElementById('live-time');
    const dateElement = document.getElementById('live-date');

    if (timeElement) {
        timeElement.textContent = timeString;
    }
    if (dateElement) {
        dateElement.textContent = dateString;
    }
}

// Update clock immediately and then every second
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', function() {
        updateClock();
        setInterval(updateClock, 1000);
    });
} else {
    updateClock();
    setInterval(updateClock, 1000);
}
