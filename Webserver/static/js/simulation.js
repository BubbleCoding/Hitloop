const cfg = {
    numScanners: 1,
    // numBeacons: 4, // Beacons are now hardcoded to corners
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
    scannerMaxSpeed: 0.1,
    scannerVelChangeMagnitude: 0.05, // For random(-val, val)
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
        this.id = id; // e.g., "Scanner-A"
        this.pos = createVector(x, y);
        this.vel = createVector(random(-cfg.scannerInitialVelMagnitude, cfg.scannerInitialVelMagnitude), 
                                random(-cfg.scannerInitialVelMagnitude, cfg.scannerInitialVelMagnitude)); // Initial velocity
        this.maxSpeed = cfg.scannerMaxSpeed; // Maximum speed
        this.size = cfg.scannerSize;
        this.colorVal = color(...cfg.scannerColor);

        // Send data at staggered intervals for each scanner
        setTimeout(() => {
            this.detectAndSendData(); // Initial send
            setInterval(() => this.detectAndSendData(), cfg.dataSendIntervalMs);
        }, Math.random() * cfg.dataSendIntervalMs); // Stagger initial sends
    }

    update() {
        // Add slight random change to velocity
        this.vel.add(createVector(random(-cfg.scannerVelChangeMagnitude, cfg.scannerVelChangeMagnitude), 
                                  random(-cfg.scannerVelChangeMagnitude, cfg.scannerVelChangeMagnitude)));
        // Limit speed
        this.vel.limit(this.maxSpeed);

        // Update position
        this.pos.add(this.vel);

        // Bounce off edges
        if (this.pos.x - this.size / 2 < 0 || this.pos.x + this.size / 2 > width) {
            this.pos.x = constrain(this.pos.x, this.size / 2, width - this.size / 2); // Prevent sticking
            this.vel.x *= -1;
        }
        if (this.pos.y - this.size / 2 < 0 || this.pos.y + this.size / 2 > height) {
            this.pos.y = constrain(this.pos.y, this.size / 2, height - this.size / 2); // Prevent sticking
            this.vel.y *= -1;
        }
    }

    detectAndSendData() {
        let payload = {
            "Scanner name": this.id,
            "beacons": {} // Initialize beacons object
        };
        let beaconsDetected = 0;

        beacons.forEach(beacon => {
            let d = dist(this.pos.x, this.pos.y, beacon.pos.x, beacon.pos.y);
            let rssi = distanceToRSSI(d);
            
            // Add beacon data to the 'beacons' object within the payload
            payload.beacons[beacon.id] = { 
                "RSSI": rssi,
                "Beacon name": beacon.name
            };
            beaconsDetected++;
        });

        if (beaconsDetected > 0) {
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
                } else {
                   // console.log(`Scanner ${this.id} sent data successfully for ${beaconsDetected} beacons.`);
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
        textAlign(CENTER, TOP); // Align text for scanner ID and RSSI list
        textSize(cfg.scannerRssiDisplayTextSize);

        // Display scanner ID
        let textYOffset = this.pos.y - this.size - 5 - cfg.scannerRssiDisplayLineHeight; // Start above the scanner
        text(this.id, this.pos.x, textYOffset);

        // Display RSSI to each beacon
        textYOffset += cfg.scannerRssiDisplayLineHeight; // Move down for the first RSSI line

        beacons.forEach(beacon => {
            let d = dist(this.pos.x, this.pos.y, beacon.pos.x, beacon.pos.y);
            let rssi = distanceToRSSI(d);
            text(`${beacon.name}: ${rssi}dBm`, this.pos.x, textYOffset);
            textYOffset += cfg.scannerRssiDisplayLineHeight; // Move to next line for next beacon
        });
    }
}

function setup() {
    createCanvas(cfg.canvasWidth, cfg.canvasHeight);
    const margin = cfg.beaconMargin; // Margin from canvas edges for beacon placement

    // Initialize Beacons in corners
    beacons.push(new Beacon(
        `beacon-NW`, "NW",
        margin, 
        margin
    ));
    beacons.push(new Beacon(
        `beacon-NE`, "NE",
        width - margin, 
        margin
    ));
    beacons.push(new Beacon(
        `beacon-SW`, "SW",
        margin, 
        height - margin
    ));
    beacons.push(new Beacon(
        `beacon-SE`, "SE",
        width - margin, 
        height - margin
    ));

    // Initialize Scanners
    for (let i = 0; i < cfg.numScanners; i++) {
        scanners.push(new Scanner(
            `scanner-${String.fromCharCode(65 + i)}`, // Scanner-A, Scanner-B, ...
            random(width), 
            random(height)
        ));
    }
    console.log("Simulation setup complete. Scanners:", scanners.length, "Beacons:", beacons.length);
}

function draw() {
    background(cfg.backgroundColor);

    beacons.forEach(b => b.display());
    scanners.forEach(s => {
        s.update();
        s.display();
    });

    // Draw distance circles from each beacon to each scanner
    noFill();
    strokeWeight(cfg.beaconDistanceCircleStrokeWeight);
    
    beacons.forEach(beacon => {
        // Set stroke color for this beacon's distance circles (can be outside scanner loop if same for all)
        stroke(...cfg.beaconDistanceCircleStrokeColor); 
        scanners.forEach(scanner => {
            let d = dist(beacon.pos.x, beacon.pos.y, scanner.pos.x, scanner.pos.y);
            ellipse(beacon.pos.x, beacon.pos.y, d * 2, d * 2); // d is radius, ellipse takes diameter
        });
    });
} 