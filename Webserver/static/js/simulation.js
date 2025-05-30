const NUM_SCANNERS = 10;
const NUM_BEACONS = 4;
const DATA_SEND_INTERVAL_MS = 5000; // Interval to send data for each scanner

let scanners = [];
let beacons = [];

// --- Helper function to simulate RSSI from distance ---
// This is a simplified model. RSSI typically ranges from -30 (very close) to -100 (far).
function distanceToRSSI(distance) {
    const maxDist = Math.sqrt(width * width + height * height); // Theoretical max distance on canvas
    if (distance < 1) distance = 1; // Avoid issues with log or zero distance
    
    // Simple linear mapping: -30 dBm at 0 distance, -100 dBm at maxDist/2
    // We'll cap distance for RSSI calculation to avoid extremely low/high values if beacons are too close/far.
    let effectiveDistance = Math.min(distance, maxDist / 2); 
    
    let rssi = map(effectiveDistance, 0, maxDist / 2, -30, -90);
    return Math.round(rssi); // Return integer RSSI
}

// --- Beacon Class ---
class Beacon {
    constructor(id, x, y) {
        this.id = id; // e.g., "Beacon-1"
        this.name = `Beacon ${id.split('-')[1]}`; // e.g., "Beacon 1" for display/API
        this.pos = createVector(x, y);
        this.radius = 10;
        this.color = color(255, 0, 0, 150); // Reddish
    }

    display() {
        fill(this.color);
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
        this.size = 15;
        this.color = color(0, 0, 255, 150); // Bluish

        // Send data at staggered intervals for each scanner
        setTimeout(() => {
            this.detectAndSendData(); // Initial send
            setInterval(() => this.detectAndSendData(), DATA_SEND_INTERVAL_MS);
        }, Math.random() * DATA_SEND_INTERVAL_MS); // Stagger initial sends
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
        fill(this.color);
        noStroke();
        rectMode(CENTER);
        rect(this.pos.x, this.pos.y, this.size, this.size);
        fill(0);
        textAlign(CENTER, CENTER);
        text(this.id, this.pos.x, this.pos.y - this.size);
    }
}

function setup() {
    createCanvas(800, 600);
    
    // Initialize Beacons
    for (let i = 0; i < NUM_BEACONS; i++) {
        beacons.push(new Beacon(
            `beacon-${i + 1}`, // Unique ID
            random(width * 0.2, width * 0.8), 
            random(height * 0.2, height * 0.8)
        ));
    }

    // Initialize Scanners
    for (let i = 0; i < NUM_SCANNERS; i++) {
        scanners.push(new Scanner(
            `scanner-${String.fromCharCode(65 + i)}`, // Scanner-A, Scanner-B, ...
            random(width), 
            random(height)
        ));
    }
    console.log("Simulation setup complete. Scanners:", scanners.length, "Beacons:", beacons.length);
}

function draw() {
    background(230);

    beacons.forEach(b => b.display());
    scanners.forEach(s => s.display());
} 