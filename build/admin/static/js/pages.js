page_to_delete = ''

function delete_page() {
    page_to_delete = decodeURI($(this).attr('data-id'));
    $('#confirm-delete-dialog').modal();
}

function confirm_delete() {
    var id = page_to_delete;
    $.post('/admin/pages/delete/' + encodeURI(id) + '/', {}, function(data, status) {
        if (data == 'success') {
            $('tr[data-id=\'' + encodeURI(id) + '\']').remove();
            $('#confirm-delete-dialog').modal('toggle');
        }
    });
}

function reorder_pages() {
    var orders = {};
    $('tr.pages').each(function () {
        var id = $(this).attr('data-id');
        var order_str = $('.order[data-id=\'' + id + '\']').val();
        var order = parseInt(order_str);
        if (order === NaN || order === null) {
            alert('invalid number.');
        }
        orders[decodeURI(id)] = order;
    });
    var orders_str = JSON.stringify(orders);
    $.post('/admin/pages/reorder/', orders_str, function(data, status) {
        if (data === 'success') {
            location.reload(true);
        }
    });
}

$(document).ready(function() {
    $('.delete-page').click(delete_page);
    $('#confirm-delete').click(confirm_delete);
    $('#re-order').click(reorder_pages);
});