function fetchAndUpdateScanners() {
    fetch('/devices')
        .then(response => response.json())
        .then(scannersData => {
            const container = document.getElementById('scanner-data-container');
            if (Object.keys(scannersData).length > 0) {
                let html = '';
                for (const [scannerId, scannerInfo] of Object.entries(scannersData)) {
                    html += `<h2>Scanner: ${scannerId} (Last Update: ${new Date(scannerInfo.timestamp).toLocaleString()})</h2>`;
                    
                    if (scannerInfo.beacons_observed && Object.keys(scannerInfo.beacons_observed).length > 0) {
                        html += '<table><thead><tr><th>Beacon ID</th><th>Beacon Name</th><th>RSSI</th><th>Est. Distance</th></tr></thead><tbody>';
                        for (const [beaconKey, beaconData] of Object.entries(scannerInfo.beacons_observed)) {
                            html += `<tr>
                                <td>${beaconKey}</td>
                                <td>${beaconData.beacon_name}</td>
                                <td>${beaconData.rssi}</td>
                                <td>${beaconData.distance}</td>
                            </tr>`;
                        }
                        html += '</tbody></table>';
                    } else {
                        html += '<p>No beacons detected by this scanner recently.</p>';
                    }
                    html += '<hr>'; // Separator between scanners
                }
                container.innerHTML = html;
            } else {
                container.innerHTML = '<p>No scanner data received yet. Make sure the simulation is running.</p>';
            }
        })
        .catch(error => {
            console.error('Error fetching scanner data:', error);
            const container = document.getElementById('scanner-data-container');
            container.innerHTML = '<p>Error loading scanner data. Check console.</p>';
        });
}

document.addEventListener('DOMContentLoaded', () => {
    fetchAndUpdateScanners();
    setInterval(fetchAndUpdateScanners, 5000); // Refresh every 5 seconds
}); 