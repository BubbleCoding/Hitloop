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
        // Flocking parameters - will be updated by UI
        this.perceptionRadius = cfg.perceptionRadius;
        this.separationForce = cfg.separationForce;
        this.alignmentForce = cfg.alignmentForce;
        this.cohesionForce = cfg.cohesionForce;
    }

    update(flock) {
        let prevPos = this.pos.copy();

        let separation = this.separate(flock);
        let alignment = this.align(flock);
        let cohesion = this.cohere(flock);

        separation.mult(this.separationForce);
        alignment.mult(this.alignmentForce);
        cohesion.mult(this.cohesionForce);

        this.vel.add(separation);
        this.vel.add(alignment);
        this.vel.add(cohesion);
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

    // Separation: steer to avoid crowding local flockmates
    separate(flock) {
        let desiredSeparation = this.size * 2; // Desired distance from neighbors
        let steer = createVector(0, 0);
        let count = 0;
        for (let other of flock) {
            let d = dist(this.pos.x, this.pos.y, other.pos.x, other.pos.y);
            if ((d > 0) && (d < desiredSeparation) && (other !== this)) {
                let diff = p5.Vector.sub(this.pos, other.pos);
                diff.normalize();
                diff.div(d); // Weight by distance
                steer.add(diff);
                count++;
            }
        }
        if (count > 0) {
            steer.div(count);
        }
        if (steer.magSq() > 0) {
            steer.normalize();
            steer.mult(this.maxSpeed);
            steer.sub(this.vel);
            steer.limit(cfg.scannerMaxForce); // Assuming maxForce is defined in cfg, or use a fixed value
        }
        return steer;
    }

    // Alignment: steer towards the average heading of local flockmates
    align(flock) {
        let steer = createVector(0, 0);
        let count = 0;
        for (let other of flock) {
            let d = dist(this.pos.x, this.pos.y, other.pos.x, other.pos.y);
            if ((d > 0) && (d < this.perceptionRadius) && (other !== this)) {
                steer.add(other.vel);
                count++;
            }
        }
        if (count > 0) {
            steer.div(count);
            steer.normalize();
            steer.mult(this.maxSpeed);
            steer.sub(this.vel);
            steer.limit(cfg.scannerMaxForce); // Assuming maxForce is defined in cfg
        }
        return steer;
    }

    // Cohesion: steer to move toward the average position of local flockmates
    cohere(flock) {
        let sum = createVector(0, 0);
        let count = 0;
        for (let other of flock) {
            let d = dist(this.pos.x, this.pos.y, other.pos.x, other.pos.y);
            if ((d > 0) && (d < this.perceptionRadius) && (other !== this)) {
                sum.add(other.pos);
                count++;
            }
        }
        if (count > 0) {
            sum.div(count);
            return this.seek(sum);
        }
        return createVector(0,0);
    }

    // A method to calculate and apply a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    seek(target) {
        let desired = p5.Vector.sub(target, this.pos);
        desired.normalize();
        desired.mult(this.maxSpeed);
        let steer = p5.Vector.sub(desired, this.vel);
        steer.limit(cfg.scannerMaxForce); // Assuming maxForce is defined in cfg
        return steer;
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