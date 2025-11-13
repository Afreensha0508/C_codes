#!/bin/bash

# Configuration
DEVICE="/dev/custom_wdt"
MAJOR_NUM=236  # <-- Use the correct Major number from your dmesg/proc/devices
MINOR_NUM=0    # <-- Use the correct Minor number
INTERVAL=4

# Function to ensure the device node exists and has correct permissions
check_and_create_device() {
    if [ ! -c "$DEVICE" ]; then
        echo "Device file $DEVICE not found. Attempting to create it..."
        # Create the character device node with the recorded Major and Minor numbers
        sudo mknod "$DEVICE" c "$MAJOR_NUM" "$MINOR_NUM"
        
        if [ $? -ne 0 ]; then
            echo "Error: Failed to create device node $DEVICE. Check if module is loaded." >&2
            exit 1
        fi
        
        # Set permissive permissions to allow the non-root user to access it
        sudo chmod 666 "$DEVICE"
        echo "Device node created with major $MAJOR_NUM."
    fi
}

# Function to run the kick loop
run_watchdog() {
    echo "Starting custom watchdog kicker. Interval: ${INTERVAL}s. Press Ctrl+C to stop."
    
    # 1. Open the device to call wdt_open() in kernel (STARTS the watchdog)
    # The exec command opens the file and assigns it to file descriptor 3
    exec 3> "$DEVICE" 
    
    # Check if the open failed (e.g., if the kernel module isn't loaded)
    if [ $? -ne 0 ]; then
        echo "Error: Failed to open device $DEVICE. Is the custom_wdt module loaded?" >&2
        exit 1
    fi
    
    # Loop to kick the watchdog
    while true; do
        # 2. Write data to FD 3 (calls wdt_write() in kernel, PINGS the watchdog)
        echo "k" >&3
        
        sleep $INTERVAL
    done
}

# Trap signals for graceful exit. Closing FD 3 calls wdt_release() (STOPS the watchdog).
cleanup() {
    # Check if file descriptor 3 is open before trying to close it
    if /usr/bin/test -t 3; then
        exec 3>&- # Close file descriptor 3
    fi
    echo "Watchdog kicker stopped and device closed."
    exit 0
}

# --- Main Execution ---

trap cleanup INT TERM

check_and_create_device # Ensure device node is ready
run_watchdog          # Start the monitoring loop
