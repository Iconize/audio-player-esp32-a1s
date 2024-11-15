document.addEventListener('DOMContentLoaded', function() {
    const volume1 = document.getElementById('volume1');
    const volume2 = document.getElementById('volume2');
    const resetBtn = document.getElementById('resetBtn'); // Ensure this matches your button ID

    if (!volume1 || !volume2) {
        console.error('Volume sliders not found');
        return;
    }

    let activeFader = null;
    let activeVolume = null;
    let lastSentVolume = null; // Track the last sent volume to avoid redundant sends
    let sendTimer = null;

    // Function to send volume data to the ESP32
    function sendVolumeData(faderId, volume) {
        const data = {
            fader: faderId,
            value: Number(volume)
        };

        fetch('/post', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data),
        })
        .then(response => response.text())
        .then(data => {
            console.log('Response from ESP32:', data);
        })
        .catch((error) => {
            console.error('Error:', error);
        });
    }

    // Function to send mouse/touch button events related to active fader
    function sendMouseButtonEvent(action) {
        if (activeFader !== null) {
            const data = {
                fader: activeFader,
                action: action
            };

            fetch('/post', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(data),
            })
            .then(response => response.text())
            .then(data => {
                console.log('Response from ESP32:', data);
            })
            .catch((error) => {
                console.error('Error:', error);
            });
        }
    }

    // Start timer to send data every 200ms, but only if the value has changed
    function startSendingData() {
        if (sendTimer === null) {
            sendTimer = setInterval(() => {
                if (activeFader !== null && activeVolume !== null && activeVolume !== lastSentVolume) {
                    sendVolumeData(activeFader, activeVolume);
                    lastSentVolume = activeVolume; // Update last sent volume
                }
            }, 100); // Send data every 100ms
        }
    }

    // Stop the data sending timer
    function stopSendingData() {
        if (sendTimer !== null) {
            clearInterval(sendTimer);
            sendTimer = null;
        }
    }

    // Handle slider input events
    function handleSliderInput(faderId, event) {
        activeFader = faderId;
        activeVolume = event.target.value;
    }

    // Mouse and Touch event listeners for both sliders
    function attachEventListeners(sliderElement, faderId) {
        // Handle both mouse and touch events for compatibility
        sliderElement.addEventListener('mousedown', function() {
            handleSliderInput(faderId, { target: { value: sliderElement.value } });
            sendMouseButtonEvent('press');
            startSendingData();
        });

        sliderElement.addEventListener('mouseup', function() {
            sendMouseButtonEvent('release');
            stopSendingData();
            activeFader = null;
            activeVolume = null;
        });

        // Touchscreen compatibility
        sliderElement.addEventListener('touchstart', function() {
            handleSliderInput(faderId, { target: { value: sliderElement.value } });
            sendMouseButtonEvent('press');
            startSendingData();
        });

        sliderElement.addEventListener('touchend', function() {
            sendMouseButtonEvent('release');
            stopSendingData();
            activeFader = null;
            activeVolume = null;
        });

        // Handle input change for the slider
        sliderElement.addEventListener('input', function(event) {
            handleSliderInput(faderId, event);
        });
    }

    // Attach event listeners to volume1 and volume2
    attachEventListeners(volume1, 1);
    attachEventListeners(volume2, 2);

    // Function to fetch the fader values from ESP32 and set the sliders
    function fetchInitialFaderValues() {
        fetch('/getFaderValues')
        .then(response => response.json())
        .then(data => {
            console.log('Initial fader values:', data);
            // Set the faders to the values fetched from ESP32
            if (data.Fader1 !== undefined) volume1.value = data.Fader1;
            if (data.Fader2 !== undefined) volume2.value = data.Fader2;
        })
        .catch(error => {
            console.error('Error fetching initial fader values:', error);
        });
    }

    // Fetch the initial values when the page is loaded
    fetchInitialFaderValues();

    // Reset function to send reset signal to ESP32
    function resetFunction() {
        console.log("Resetting ESP32...");
    
        fetch('/reset', {
            method: 'POST',
        })
        .then(response => response.text())
        .then(data => {
            console.log('Response from ESP32:', data);
        })
        .catch((error) => {
            console.error('Error:', error);
        });
    }

    document.getElementById('resetBtn').onclick = resetFunction;

});
