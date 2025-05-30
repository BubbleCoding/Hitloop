// Scanner Class Definition
// Depends on p5.js functions (createVector, color, random, dist, fill, noStroke, rectMode, rect, textAlign, text, constrain)
// and cfg object (from config.js), distanceToRSSI (from utils.js), global beacons array.
class Scanner {
    constructor(id, x, y) {
        this.id = id;
        this.pos = createVector(x, y);
        this.lastPosForMovement = this.pos.copy();
        this.movementSinceLastSend = 0;
        this.vel = createVector(random(-cfg.scannerInitialVelMagnitude, cfg.scannerInitialVelMagnitude), 
                                random(-cfg.scannerInitialVelMagnitude, cfg.scannerInitialVelMagnitude));
        this.maxSpeed = cfg.scannerMaxSpeed;
        this.size = cfg.scannerSize;
        this.colorVal = color(...cfg.scannerColor);
    }

    update() {
        let prevPos = this.pos.copy();
        this.vel.add(createVector(random(-cfg.scannerVelChangeMagnitude, cfg.scannerVelChangeMagnitude), 
                                  random(-cfg.scannerVelChangeMagnitude, cfg.scannerVelChangeMagnitude)));
        this.vel.limit(this.maxSpeed);
        this.pos.add(this.vel);
        this.movementSinceLastSend += p5.Vector.dist(this.pos, prevPos);

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
            "movement": parseFloat(this.movementSinceLastSend.toFixed(2)),
            "beacons": {}
        };
        this.movementSinceLastSend = 0;

        let beaconsDetected = 0;
        // Assumes global 'beacons' array is populated
        beacons.forEach(beacon => {
            let d = dist(this.pos.x, this.pos.y, beacon.pos.x, beacon.pos.y);
            let rssi = distanceToRSSI(d); // Assumes distanceToRSSI is globally available from utils.js
            payload.beacons[beacon.id] = { 
                "RSSI": rssi,
                "Beacon name": beacon.name
            };
            beaconsDetected++;
        });

        if (beaconsDetected > 0 || payload.hasOwnProperty('movement')) {
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

        // Assumes global 'beacons' array is populated
        beacons.forEach(beacon => {
            let d = dist(this.pos.x, this.pos.y, beacon.pos.x, beacon.pos.y);
            let rssi = distanceToRSSI(d); // Assumes distanceToRSSI is globally available from utils.js
            text(`${beacon.name}: ${rssi}dBm`, this.pos.x, textYOffset);
            textYOffset += cfg.scannerRssiDisplayLineHeight;
        });
    }
} 