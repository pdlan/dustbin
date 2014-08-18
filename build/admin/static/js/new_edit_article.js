tags = []

function refresh_tags() {
    var html = '';
    for (var i = 0; i < tags.length; ++i) {
        html += '<span class="tag label label-default">' + tags[i] +
                '<i class="fa fa-times remove-tag" data-tag="' +
                encodeURI(tags[i]) + '"></i></span>';
    }
    $('#tags-view').html(html);
    $('.remove-tag').click(remove_tag);
    $('#tags').val(JSON.stringify(tags));
}

function remove_tag() {
    var tag = decodeURI($(this).attr('data-tag'));
    for (var i = 0; i < tags.length; ++i) {
        if (tags[i] === tag) {
            tags.splice(i, 1);
        }
    }
    refresh_tags();
}

function add_tag() {
    var new_tag = $('#new-tag').val();
    for (var i = 0; i < tags.length; ++i) {
        if (tags[i] === new_tag) {
            return;
        }
    }
    tags.push(new_tag);
    refresh_tags();
}

$(document).ready(function() {
    $('#add-tag').click(add_tag);
    refresh_tags();
});