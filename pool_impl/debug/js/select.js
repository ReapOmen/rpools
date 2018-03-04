$(document).ready(function() {
    // the X button 'closes' the chart dialog
    $('#chart-dialog > span').on('click', function() {
        $(this).parent().hide(0);
    });
    // sort number columns as integers
    $('thead th.number').data('sortBy', function(th, td, tablesort) {
	return parseInt(td.text(), 10);
    });
    $('#main-table').tablesort();
    $('#main-table > tbody > tr').each(function() {
        // clicking on a table row will generate a graph
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

/**
 *  Hides all rows whose columns do not match the given strings.
 *  Example: strs = ['a', 'b']; cols = [1, 3];
 *  Hide all rows which do not cotain 'a' in column 1 AND 'b' in column 3.
 *  @param strs the strings which are used in the filtering process
 *  @param cols the columns which are filtered
 */
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

/**
 *  Creates a chart for the given type.
 *  @param name the name of the type
 *  @param allocSize the size of the allocation
 */
function showGraph(name, allocSize) {
    var datapoints = [];
    var labels = [];
    // find all rows which refer to the given type
    $('#main-table > tbody  > tr').each(function() {
        if ($(this).find('th:eq(1)').text() === name &&
            $(this).find('th:eq(3)').text() === allocSize) {
            var snapshot = parseInt($(this).find('th:eq(0)').text(), 10);
            labels.push('Snapshot ' + snapshot + '(' + snapshot * 100 + 'ms)');
            datapoints.push(parseInt($(this).find('th:eq(4)').text(), 10));
        }
    });
    // clear the last chart and remove it in order to create a new one
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
