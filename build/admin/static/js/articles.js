article_to_delete = ''

function delete_article() {
    article_to_delete = decodeURI($(this).attr('data-id'));
    $('#confirm-delete-dialog').modal();
}

function confirm_delete() {
    var id = article_to_delete;
    $.post('/admin/articles/delete/' + encodeURI(id) + '/', {}, function(data, status) {
        if (data == 'success') {
            $('tr[data-id=\'' + encodeURI(id) + '\']').remove();
            $('#confirm-delete-dialog').modal('toggle');
        }
    });
}

$(document).ready(function() {
    $('.delete-article').click(delete_article);
    $('#confirm-delete').click(confirm_delete);
});