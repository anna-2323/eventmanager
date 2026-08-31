let chartInstances = {};

export function createChart(canvasId, type, labels, data, label) {
    const canvas = document.getElementById(canvasId);

    if (!canvas) {
        console.error(`graph.js: #${canvasId} not found`);
        return null;
    }

    if (chartInstances[canvasId]) {
        chartInstances[canvasId].destroy();
    }

    chartInstances[canvasId] = new Chart(canvas, {
        type: type,

        data: {
            labels: labels,
            datasets: [{
                label: label,
                data: data,
                borderWidth: 2,
                tension: 0.3,
                fill: type === "line"
            }]
        },

        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    display: true
                }
            },
            scales: type === "pie" || type === "doughnut"
                ? {}
                : {
                    y: {
                        beginAtZero: true
                    }
                }
        }
    });

    return chartInstances[canvasId];
}

export function createLineChart(canvasId, labels, data, label) {
    return createChart(
        canvasId,
        "line",
        labels,
        data,
        label
    );
}

export function createBarChart(canvasId, labels, data, label) {
    return createChart(
        canvasId,
        "bar",
        labels,
        data,
        label
    );
}

export function createPieChart(canvasId, labels, data, label) {
    return createChart(
        canvasId,
        "pie",
        labels,
        data,
        label
    );
}

export function createMultiLineChart(canvasId, labels, datasets) {
    const canvas = document.getElementById(canvasId);

    if (!canvas) {
        console.error(`graph.js: #${canvasId} not found`);
        return null;
    }

    if (chartInstances[canvasId]) {
        chartInstances[canvasId].destroy();
    }

    chartInstances[canvasId] = new Chart(canvas, {
        type: "line",

        data: {
            labels: labels,
            datasets: datasets.map(dataset => ({
                label: dataset.label,
                data: dataset.data,
                borderWidth: 2,
                tension: 0.3,
                fill: false
            }))
        },

        options: {
            responsive: true,
            maintainAspectRatio: false,

            plugins: {
                legend: {
                    display: true
                }
            },

            scales: {
                y: {
                    beginAtZero: true
                }
            }
        }
    });

    return chartInstances[canvasId];
}