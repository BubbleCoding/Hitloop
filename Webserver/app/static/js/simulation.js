// Main simulation logic file: simulation.js
// Depends on: config.js, utils.js, Beacon.js, Scanner.js
// and p5.js (core & dom)

// Global variables for the simulation entities and canvas
let scanners = [];
let beacons = [];
let canvas;

// UI Element Variables (sliders and their labels)
let numScannersSlider, maxSpeedSlider, velChangeSlider;
let numScannersLabel, maxSpeedLabel, velChangeLabel;
// Flocking UI Elements
let perceptionRadiusSlider, separationForceSlider, alignmentForceSlider, cohesionForceSlider;
let perceptionRadiusLabel, separationForceLabel, alignmentForceLabel, cohesionForceLabel;

// Initialization function for beacons (calls Beacon class from Beacon.js)
function initializeBeacons() {
    beacons.length = 0; 
    const margin = cfg.beaconMargin;
    beacons.push(new Beacon(`beacon-NW`, "NW", margin, margin));
    beacons.push(new Beacon(`beacon-NE`, "NE", cfg.canvasWidth - margin, margin));
    beacons.push(new Beacon(`beacon-SW`, "SW", margin, cfg.canvasHeight - margin));
    beacons.push(new Beacon(`beacon-SE`, "SE", cfg.canvasWidth - cfg.beaconMargin, cfg.canvasHeight - margin));
}

// Initialization function for scanners (calls Scanner class from Scanner.js)
function initializeScanners() {
    scanners.length = 0; 
    for (let i = 0; i < cfg.numScanners; i++) {
        scanners.push(new Scanner(
            `scanner-${String.fromCharCode(65 + i)}`,
            random(cfg.canvasWidth),
            random(cfg.canvasHeight)
        ));
    }
}

// Function to reset simulation state and respawn scanners
function resetAndRespawnScanners() {
    cfg.numScanners = numScannersSlider.value();
    numScannersLabel.html(`Number of Scanners: ${cfg.numScanners}`);

    fetch('/reset_devices', { method: 'GET' })
        .then(response => response.json())
        .then(data => {
            console.log('Reset devices API response:', data);
        })
        .catch(error => console.error('Error resetting devices:', error));
    
    initializeScanners(); 
    sendAllScannersData(); 
}

// Initialization function for UI elements (sliders, labels)
function initializeUI() {
    const uiPanel = select('#ui-controls-panel');

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

    // Flocking UI controls
    let perceptionRadiusDiv = createDiv();
    perceptionRadiusDiv.parent(uiPanel);
    perceptionRadiusDiv.addClass('control-group');
    perceptionRadiusLabel = createSpan(`Perception Radius: ${cfg.perceptionRadius}`);
    perceptionRadiusLabel.parent(perceptionRadiusDiv);
    perceptionRadiusSlider = createSlider(cfg.minPerceptionRadius, cfg.maxPerceptionRadius, cfg.perceptionRadius, cfg.perceptionRadiusStep);
    perceptionRadiusSlider.parent(perceptionRadiusDiv);
    perceptionRadiusSlider.input(() => {
        cfg.perceptionRadius = perceptionRadiusSlider.value();
        perceptionRadiusLabel.html(`Perception Radius: ${cfg.perceptionRadius}`);
        scanners.forEach(s => s.perceptionRadius = cfg.perceptionRadius);
    });

    let separationForceDiv = createDiv();
    separationForceDiv.parent(uiPanel);
    separationForceDiv.addClass('control-group');
    separationForceLabel = createSpan(`Separation Force: ${cfg.separationForce.toFixed(2)}`);
    separationForceLabel.parent(separationForceDiv);
    separationForceSlider = createSlider(cfg.minSeparationForce, cfg.maxSeparationForce, cfg.separationForce, cfg.separationForceStep);
    separationForceSlider.parent(separationForceDiv);
    separationForceSlider.input(() => {
        cfg.separationForce = separationForceSlider.value();
        separationForceLabel.html(`Separation Force: ${cfg.separationForce.toFixed(2)}`);
        scanners.forEach(s => s.separationForce = cfg.separationForce);
    });

    let alignmentForceDiv = createDiv();
    alignmentForceDiv.parent(uiPanel);
    alignmentForceDiv.addClass('control-group');
    alignmentForceLabel = createSpan(`Alignment Force: ${cfg.alignmentForce.toFixed(2)}`);
    alignmentForceLabel.parent(alignmentForceDiv);
    alignmentForceSlider = createSlider(cfg.minAlignmentForce, cfg.maxAlignmentForce, cfg.alignmentForce, cfg.alignmentForceStep);
    alignmentForceSlider.parent(alignmentForceDiv);
    alignmentForceSlider.input(() => {
        cfg.alignmentForce = alignmentForceSlider.value();
        alignmentForceLabel.html(`Alignment Force: ${cfg.alignmentForce.toFixed(2)}`);
        scanners.forEach(s => s.alignmentForce = cfg.alignmentForce);
    });

    let cohesionForceDiv = createDiv();
    cohesionForceDiv.parent(uiPanel);
    cohesionForceDiv.addClass('control-group');
    cohesionForceLabel = createSpan(`Cohesion Force: ${cfg.cohesionForce.toFixed(2)}`);
    cohesionForceLabel.parent(cohesionForceDiv);
    cohesionForceSlider = createSlider(cfg.minCohesionForce, cfg.maxCohesionForce, cfg.cohesionForce, cfg.cohesionForceStep);
    cohesionForceSlider.parent(cohesionForceDiv);
    cohesionForceSlider.input(() => {
        cfg.cohesionForce = cohesionForceSlider.value();
        cohesionForceLabel.html(`Cohesion Force: ${cfg.cohesionForce.toFixed(2)}`);
        scanners.forEach(s => s.cohesionForce = cfg.cohesionForce);
    });
}

// Global function to trigger data sending for all scanners
function sendAllScannersData() {
    scanners.forEach(scanner => {
        if (scanner && typeof scanner.detectAndSendData === 'function') {
            scanner.detectAndSendData();
        }
    });
}

// p5.js setup function: Initializes canvas, beacons, scanners, UI, and data sending interval
function setup() {
    canvas = createCanvas(cfg.canvasWidth, cfg.canvasHeight);
    let canvasParentDiv = document.getElementById('canvas-parent');
    if (canvasParentDiv) {
        canvas.parent(canvasParentDiv);
    } else {
        console.error("CRITICAL ERROR: Could not find div 'canvas-parent' for p5.js canvas.");
    }

    initializeBeacons();
    initializeScanners(); 
    initializeUI();
    
    sendAllScannersData(); 
    setInterval(sendAllScannersData, cfg.dataSendIntervalMs);
    
    console.log("Simulation setup complete. Scanners:", scanners.length, "Beacons:", beacons.length);
}

// p5.js draw function: Called every frame to render the simulation
function draw() {
    background(cfg.backgroundColor);

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
        s.update(scanners);
        s.display();
    });
}

// p5.js mousePressed function: Handles mouse clicks on the canvas
function mousePressed() {
    if (mouseX >= 0 && mouseX <= cfg.canvasWidth && mouseY >= 0 && mouseY <= cfg.canvasHeight) {
        if (scanners.length > 0 && scanners[0]) {
            scanners[0].pos.x = mouseX;
            scanners[0].pos.y = mouseY;
        }
    }
} 