// Beacon Class Definition
// Depends on p5.js functions (createVector, color, fill, noStroke, ellipse, textAlign, text)
// and cfg object (from config.js)
class Beacon {
    constructor(id, name, x, y) {
        this.id = id;
        this.name = name;
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