function fetchAndUpdateScanners() {
    fetch('/devices')
        .then(response => response.json())
        .then(scannersData => {
            const container = document.getElementById('scanner-data-container');
            
            if (Object.keys(scannersData).length === 0) {
                container.innerHTML = '<p>No scanner data received yet. Make sure the simulation is running.</p>';
                return;
            }

            // Dynamically determine beacon columns from the data
            const beaconColumnMap = new Map(); // Use a Map to store unique beaconId -> beaconName
            for (const scannerId in scannersData) {
                const scannerInfo = scannersData[scannerId];
                if (scannerInfo.beacons_observed) {
                    for (const beaconId in scannerInfo.beacons_observed) {
                        if (!beaconColumnMap.has(beaconId)) {
                            beaconColumnMap.set(beaconId, scannerInfo.beacons_observed[beaconId].beacon_name);
                        }
                    }
                }
            }

            // Convert map to an array and sort for consistent column order
            // Sorting by beaconId (e.g., 'beacon-NE', 'beacon-NW', 'beacon-SE', 'beacon-SW')
            const sortedBeaconColumns = Array.from(beaconColumnMap.entries())
                .map(([id, name]) => ({ id, name }))
                .sort((a, b) => a.id.localeCompare(b.id));

            let tableHtml = '<table><thead><tr><th>Scanner Name</th>';
            
            // Add beacon headers from our dynamically generated and sorted list
            sortedBeaconColumns.forEach(beaconCol => {
                tableHtml += `<th>${beaconCol.name} (RSSI)</th>`;
            });
            tableHtml += '<th>Last Update</th></tr></thead><tbody>'; // Added Last Update column for scanner

            // Add scanner rows
            for (const scannerId of Object.keys(scannersData).sort()) { // Sort scanner IDs for consistent row order
                const scannerInfo = scannersData[scannerId];
                tableHtml += `<tr><td>${scannerId}</td>`;
                
                sortedBeaconColumns.forEach(beaconCol => {
                    let rssiValue = '-'; // Default if not found for this scanner
                    if (scannerInfo.beacons_observed && scannerInfo.beacons_observed[beaconCol.id]) {
                        rssiValue = scannerInfo.beacons_observed[beaconCol.id].rssi;
                    }
                    tableHtml += `<td>${rssiValue}</td>`;
                });
                // Add scanner's last update time
                tableHtml += `<td>${new Date(scannerInfo.timestamp).toLocaleTimeString()}</td>`;
                tableHtml += '</tr>';
            }
            tableHtml += '</tbody></table>';
            container.innerHTML = tableHtml;

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