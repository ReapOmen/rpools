$(document).ready(function() {
    // sort number columns as integers
    $('thead th.number').data('sortBy', function(th, td, tablesort) {
	return parseInt(td.text(), 16);
    });
    $('#main-table').tablesort();
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
