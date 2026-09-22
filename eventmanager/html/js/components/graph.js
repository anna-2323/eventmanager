let chartInstances = {};

export function createChart(canvasId, type, labels, datasets, integers = false) {
    const canvas = document.getElementById(canvasId);

    if (!canvas) {
        console.error(`graph.js: #${canvasId} not found`);
        return null;
    }

    if (chartInstances[canvasId]) {
        chartInstances[canvasId].destroy();
    }

    const yTicks = integers
        ? {
            stepSize: 1,
            precision: 0
        }
        : {};

    chartInstances[canvasId] = new Chart(canvas, {
        type: type,

        data: {
            labels: labels,
            datasets: datasets
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
                        beginAtZero: true,
                        ticks: yTicks
                    }
                }
        }
    });

    return chartInstances[canvasId];
}

export function createLineChart(canvasId, labels, data, label, integers = false) {
    const datasets = [{
                label: label,
                data: data,
                borderWidth: 2,
                tension: 0.3,
                fill: true
            }];
    return createChart(
        canvasId,
        "line",
        labels,
        datasets,
        integers
    );
}

export function createBarChart(canvasId, labels, data, label, integers = false) {
    const datasets = [{
                label: label,
                data: data,
                borderWidth: 2,
                tension: 0.3,
                fill: false
            }];
    return createChart(
        canvasId,
        "bar",
        labels,
        datasets,
        integers
    );
}

export function createPieChart(canvasId, labels, data, label, integers = false) {
    const datasets = [{
                label: label,
                data: data,
                borderWidth: 2,
                tension: 0.3,
                fill: false
            }];
    return createChart(
        canvasId,
        "pie",
        labels,
        datasets,
        integers
    );
}

export function createMultiLineChart(canvasId, labels, datasets, integers = false) {
    datasets = datasets.map(dataset => ({
                label: dataset.label,
                data: dataset.data,
                borderWidth: 2,
                tension: 0.3,
                fill: false
            }));
    return createChart(
        canvasId,
        "line",
        labels,
        datasets,
        integers
    );
}
