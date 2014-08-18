article_to_delete = ''

function delete_article() {
    article_to_delete = $(this).attr('data-id');
    $('#confirm-delete-dialog').modal();
}

function confirm_article() {
    var id = article_to_delete;
    $.post('/admin/articles/delete/' + id + '/', {}, function(data, status) {
        if (data == 'success') {
            $('tr[data-id=\'' + id + '\']').remove();
            $('#confirm-delete-dialog').modal('toggle');
        }
    });
}

$(document).ready(function() {
    $('.delete-article').click(delete_article);
    $('#confirm-delete').click(confirm_article);
});