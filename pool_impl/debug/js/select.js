$(document).ready(function() {
    $('#chart-dialog > span').on('click', function() {
        $(this).parent().hide(0);
    });
    // sort number columns as integers
    $('thead th.number').data('sortBy', function(th, td, tablesort) {
	return parseInt(td.text(), 10);
    });
    $('#main-table').tablesort();
    $('#main-table > tbody > tr').each(function() {
        $(this).on('click', function () {
            var name = $(this).find('th:eq(1)').text();
            var allocSize = $(this).find('th:eq(3)').text();
            showGraph(name, allocSize);
        });
    });
    var typeFilter = $('#type-filter');
    var functionFilter = $('#function-filter');
    typeFilter.on('input', function() {
        filterByColumn([typeFilter.val(), functionFilter.val()], [1, 6]);
    });
    functionFilter.on('input', function() {
        filterByColumn([typeFilter.val(), functionFilter.val()], [1, 6]);
    });
});

function filterByColumn(strs, cols) {
    $('#main-table > tbody  > tr').each(function() {
        var show = true;
        for (var i = 0; i < 2; ++i) {
            var column = $(this).find('th:eq(' + cols[i] + ')').text();
            if (column.indexOf(strs[i]) == -1) {
                show = false;
                break;
            }
        }
        if (show) {
            $(this).css('display', '');
        } else {
            $(this).css('display', 'none');
        }
    });
};

var chart = undefined;

function showGraph(name, allocSize) {
    var datapoints = [];
    var labels = [];
    $('#main-table > tbody  > tr').each(function() {
        if ($(this).find('th:eq(1)').text() === name &&
            $(this).find('th:eq(3)').text() === allocSize) {
            var snapshot = parseInt($(this).find('th:eq(0)').text(), 10);
            labels.push('Snapshot ' + snapshot + '(' + snapshot * 100 + 'ms)');
            datapoints.push(parseInt($(this).find('th:eq(4)').text(), 10));
        }
    });
    if (chart !== undefined) {
        chart.clear();
        chart.destroy();
    }
    chart = new Chart($('#chart-canvas'), {
        type: 'bar',
        data: {
            labels: labels,
            datasets: [
                {
                    label: name + ' (size:' + allocSize + ')',
                    borderWidth: 1,
                    backgroundColor: "rgba(255,215,0,1)",
                    data: datapoints
                }
            ]
        },
        options: {
            scales: {
                yAxes: [{
                    scaleLabel: {
                        display: true,
                        labelString: 'Allocations'
}
                }]
            }
        }
    });
    $('#chart-dialog').show(0);
};
