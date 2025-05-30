const cfg = {
    numScanners: 10, // Initial number of scanners
    minScanners: 1,
    maxScanners: 50,
    scannerStep: 1,

    dataSendIntervalMs: 5000,

    canvasWidth: 800,
    canvasHeight: 600,
    backgroundColor: 230,

    beaconRadius: 10,
    beaconColor: [255, 0, 0, 150], // [R, G, B, Alpha]
    beaconMargin: 30,
    beaconDistanceCircleStrokeColor: [100, 100, 100, 70], // Semi-transparent gray
    beaconDistanceCircleStrokeWeight: 1,

    scannerSize: 15,
    scannerColor: [0, 0, 255, 150], // [R, G, B, Alpha]
    scannerInitialVelMagnitude: 1, // For random(-val, val)
    
    scannerMaxSpeed: 0.1, // Initial max speed
    minScannerMaxSpeed: 0.1,
    maxScannerMaxSpeed: 5,
    scannerMaxSpeedStep: 0.1,

    scannerVelChangeMagnitude: 0.05, // Initial vel change magnitude
    minScannerVelChangeMag: 0.01,
    maxScannerVelChangeMag: 0.5,
    scannerVelChangeMagStep: 0.01,

    scannerRssiDisplayTextColor: [0, 0, 0], // Color for RSSI text
    scannerRssiDisplayTextSize: 10,
    scannerRssiDisplayLineHeight: 12,

    // RSSI simulation parameters
    rssiAtMinDist: -30,         // RSSI at very close distance (e.g., 0-1 units)
    rssiAtEffectiveMaxDist: -90, // RSSI at an effective maximum calculation distance
    // effectiveDistance is Math.min(distance, maxDist / 2)
    // The '0' in map(effectiveDistance, 0, maxDist/2, ...) refers to the start of the input range for map.
};

// const NUM_SCANNERS = 10; // Moved to cfg.numScanners
// const DATA_SEND_INTERVAL_MS = 5000; // Moved to cfg.dataSendIntervalMs

let scanners = [];
let beacons = [];
let canvas;

// UI Element Variables
let numScannersSlider, maxSpeedSlider, velChangeSlider;
let numScannersLabel, maxSpeedLabel, velChangeLabel;

function initializeBeacons() {
    beacons.length = 0; // Clear existing beacons
    const margin = cfg.beaconMargin;
    beacons.push(new Beacon(`beacon-NW`, "NW", margin, margin));
    beacons.push(new Beacon(`beacon-NE`, "NE", cfg.canvasWidth - margin, margin));
    beacons.push(new Beacon(`beacon-SW`, "SW", margin, cfg.canvasHeight - margin));
    beacons.push(new Beacon(`beacon-SE`, "SE", cfg.canvasWidth - cfg.beaconMargin, cfg.canvasHeight - margin));
}

function initializeScanners() {
    scanners.length = 0; // Clear existing scanners before creating new ones
    for (let i = 0; i < cfg.numScanners; i++) {
        scanners.push(new Scanner(
            `scanner-${String.fromCharCode(65 + i)}`,
            random(cfg.canvasWidth),
            random(cfg.canvasHeight)
        ));
    }
}

function resetAndRespawnScanners() {
    cfg.numScanners = numScannersSlider.value();
    numScannersLabel.html(`Number of Scanners: ${cfg.numScanners}`);

    fetch('/reset_devices', { method: 'GET' })
        .then(response => response.json())
        .then(data => {
            console.log('Reset devices API response:', data);
            // initializeScanners(); // Was previously here in some versions, now called below
        })
        .catch(error => console.error('Error resetting devices:', error));
    
    initializeScanners(); // Creates new scanner instances
    sendAllScannersData(); // Immediately send data for newly created scanners
}

function initializeUI() {
    const uiPanel = select('#ui-controls-panel');

    // --- Number of Scanners Slider ---
    let scannerDiv = createDiv();
    scannerDiv.parent(uiPanel);
    scannerDiv.addClass('control-group');
    numScannersLabel = createSpan(`Number of Scanners: ${cfg.numScanners}`);
    numScannersLabel.parent(scannerDiv);
    numScannersSlider = createSlider(cfg.minScanners, cfg.maxScanners, cfg.numScanners, cfg.scannerStep);
    numScannersSlider.parent(scannerDiv);
    numScannersSlider.input(() => {
        cfg.numScanners = numScannersSlider.value();
        numScannersLabel.html(`Number of Scanners: ${cfg.numScanners}`);
    });
    numScannersSlider.changed(resetAndRespawnScanners);

    // --- Max Speed Slider ---
    let maxSpeedDiv = createDiv();
    maxSpeedDiv.parent(uiPanel);
    maxSpeedDiv.addClass('control-group');
    maxSpeedLabel = createSpan(`Scanner Max Speed: ${cfg.scannerMaxSpeed.toFixed(2)}`);
    maxSpeedLabel.parent(maxSpeedDiv);
    maxSpeedSlider = createSlider(cfg.minScannerMaxSpeed, cfg.maxScannerMaxSpeed, cfg.scannerMaxSpeed, cfg.scannerMaxSpeedStep);
    maxSpeedSlider.parent(maxSpeedDiv);
    maxSpeedSlider.input(() => {
        cfg.scannerMaxSpeed = maxSpeedSlider.value();
        maxSpeedLabel.html(`Scanner Max Speed: ${cfg.scannerMaxSpeed.toFixed(2)}`);
        scanners.forEach(s => s.maxSpeed = cfg.scannerMaxSpeed);
    });

    // --- Velocity Change Magnitude Slider ---
    let velChangeDiv = createDiv();
    velChangeDiv.parent(uiPanel);
    velChangeDiv.addClass('control-group');
    velChangeLabel = createSpan(`Vel. Change Mag.: ${cfg.scannerVelChangeMagnitude.toFixed(2)}`);
    velChangeLabel.parent(velChangeDiv);
    velChangeSlider = createSlider(cfg.minScannerVelChangeMag, cfg.maxScannerVelChangeMag, cfg.scannerVelChangeMagnitude, cfg.scannerVelChangeMagStep);
    velChangeSlider.parent(velChangeDiv);
    velChangeSlider.input(() => {
        cfg.scannerVelChangeMagnitude = velChangeSlider.value();
        velChangeLabel.html(`Vel. Change Mag.: ${cfg.scannerVelChangeMagnitude.toFixed(2)}`);
    });
}

// --- Helper function to simulate RSSI from distance ---
// This is a simplified model. RSSI typically ranges from -30 (very close) to -100 (far).
function distanceToRSSI(distance) {   
    // Use configured canvas dimensions for consistency
    const maxDist = Math.sqrt(cfg.canvasWidth * cfg.canvasWidth + cfg.canvasHeight * cfg.canvasHeight);
    if (distance < 1) distance = 1; // Avoid issues with log or zero distance
    
    // Simple linear mapping: -30 dBm at 0 distance, -100 dBm at maxDist/2
    // We'll cap distance for RSSI calculation to avoid extremely low/high values if beacons are too close/far.
    let effectiveDistance = Math.min(distance, maxDist / 2); 
    
    let rssi = map(effectiveDistance, 0, maxDist / 2, cfg.rssiAtMinDist, cfg.rssiAtEffectiveMaxDist);
    return Math.round(rssi); // Return integer RSSI
    
}

// --- Beacon Class ---
class Beacon {
    constructor(id, name, x, y) {
        this.id = id; // e.g., "beacon-NW"
        this.name = name; // e.g., "NW"
        this.pos = createVector(x, y);
        this.radius = cfg.beaconRadius;
        this.colorVal = color(...cfg.beaconColor);
    }

    display() {
        fill(this.colorVal);
        noStroke();
        ellipse(this.pos.x, this.pos.y, this.radius * 2);
        fill(0);
        textAlign(CENTER, CENTER);
        text(this.id, this.pos.x, this.pos.y - this.radius - 5);
    }
}

// --- Scanner Class ---
class Scanner {
    constructor(id, x, y) {
        this.id = id;
        this.pos = createVector(x, y);
        this.lastPosForMovement = this.pos.copy(); // Store initial position for movement calc
        this.movementSinceLastSend = 0;
        this.vel = createVector(random(-cfg.scannerInitialVelMagnitude, cfg.scannerInitialVelMagnitude), 
                                random(-cfg.scannerInitialVelMagnitude, cfg.scannerInitialVelMagnitude));
        this.maxSpeed = cfg.scannerMaxSpeed;
        this.size = cfg.scannerSize;
        this.colorVal = color(...cfg.scannerColor);
    }

    update() {
        let prevPos = this.pos.copy(); // Position before current frame's movement

        this.vel.add(createVector(random(-cfg.scannerVelChangeMagnitude, cfg.scannerVelChangeMagnitude), 
                                  random(-cfg.scannerVelChangeMagnitude, cfg.scannerVelChangeMagnitude)));
        this.vel.limit(this.maxSpeed);
        this.pos.add(this.vel);

        // Accumulate distance moved this frame
        this.movementSinceLastSend += p5.Vector.dist(this.pos, prevPos);

        // Bounce off edges
        if (this.pos.x - this.size / 2 < 0 || this.pos.x + this.size / 2 > cfg.canvasWidth) {
            this.pos.x = constrain(this.pos.x, this.size / 2, cfg.canvasWidth - this.size / 2);
            this.vel.x *= -1;
        }
        if (this.pos.y - this.size / 2 < 0 || this.pos.y + this.size / 2 > cfg.canvasHeight) {
            this.pos.y = constrain(this.pos.y, this.size / 2, cfg.canvasHeight - this.size / 2);
            this.vel.y *= -1;
        }
    }

    detectAndSendData() {
        let payload = {
            "Scanner name": this.id,
            "movement": parseFloat(this.movementSinceLastSend.toFixed(2)), // Include movement
            "beacons": {}
        };
        // Reset movement for the next interval AFTER it's added to payload
        this.movementSinceLastSend = 0;
        // this.lastPosForMovement = this.pos.copy(); // Update lastPos after sending data

        let beaconsDetected = 0;
        beacons.forEach(beacon => {
            let d = dist(this.pos.x, this.pos.y, beacon.pos.x, beacon.pos.y);
            let rssi = distanceToRSSI(d);
            payload.beacons[beacon.id] = { 
                "RSSI": rssi,
                "Beacon name": beacon.name
            };
            beaconsDetected++;
        });

        if (beaconsDetected > 0 || payload.hasOwnProperty('movement')) { // Send if beacons or just movement data
            fetch('/data', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(payload),
            })
            .then(response => response.json())
            .then(data => {
                if (data.status !== 'success') {
                    console.error(`Scanner ${this.id} failed to send data:`, data.message);
                }
            })
            .catch((error) => {
                console.error(`Scanner ${this.id} error sending data:`, error);
            });
        }
    }

    display() {
        fill(this.colorVal);
        noStroke();
        rectMode(CENTER);
        rect(this.pos.x, this.pos.y, this.size, this.size);
        
        fill(...cfg.scannerRssiDisplayTextColor);
        textAlign(CENTER, TOP);
        textSize(cfg.scannerRssiDisplayTextSize);

        let textYOffset = this.pos.y - this.size - 5 - cfg.scannerRssiDisplayLineHeight;
        text(this.id, this.pos.x, textYOffset);

        textYOffset += cfg.scannerRssiDisplayLineHeight;

        beacons.forEach(beacon => {
            let d = dist(this.pos.x, this.pos.y, beacon.pos.x, beacon.pos.y);
            let rssi = distanceToRSSI(d);
            text(`${beacon.name}: ${rssi}dBm`, this.pos.x, textYOffset);
            textYOffset += cfg.scannerRssiDisplayLineHeight;
        });
    }
}

function sendAllScannersData() {
    // console.log(`Globally sending data for ${scanners.length} scanners at ${new Date().toLocaleTimeString()}`);
    scanners.forEach(scanner => {
        if (scanner && typeof scanner.detectAndSendData === 'function') {
            scanner.detectAndSendData();
        }
    });
}

function setup() {
    canvas = createCanvas(cfg.canvasWidth, cfg.canvasHeight);
    
    // Parent the canvas to the div#canvas-parent
    let canvasParentDiv = document.getElementById('canvas-parent');
    if (canvasParentDiv) {
        canvas.parent(canvasParentDiv);
    } else {
        console.error("CRITICAL ERROR: Could not find the div with id 'canvas-parent' to attach the p5.js canvas. UI might not work as expected.");
    }

    initializeBeacons();
    initializeScanners(); // Initial scanner setup
    initializeUI();
    
    // Initial data send for all scanners
    sendAllScannersData(); 
    // Global interval to send data for all scanners
    setInterval(sendAllScannersData, cfg.dataSendIntervalMs);
    
    console.log("Simulation setup complete. Scanners:", scanners.length, "Beacons:", beacons.length);
}

function draw() {
    background(cfg.backgroundColor);

    // Draw distance circles from each beacon to each scanner
    noFill();
    strokeWeight(cfg.beaconDistanceCircleStrokeWeight);
    beacons.forEach(beacon => {
        stroke(...cfg.beaconDistanceCircleStrokeColor); 
        scanners.forEach(scanner => {
            let d = dist(beacon.pos.x, beacon.pos.y, scanner.pos.x, scanner.pos.y);
            ellipse(beacon.pos.x, beacon.pos.y, d * 2, d * 2);
        });
    });
    
    beacons.forEach(b => b.display());
    scanners.forEach(s => {
        s.update();
        s.display();
    });
}

function mousePressed() {
    if (mouseX >= 0 && mouseX <= cfg.canvasWidth && mouseY >= 0 && mouseY <= cfg.canvasHeight) {
        if (scanners.length > 0 && scanners[0]) {
            scanners[0].pos.x = mouseX;
            scanners[0].pos.y = mouseY;
            // scanners[0].vel.mult(0); 
        }
    }
} 