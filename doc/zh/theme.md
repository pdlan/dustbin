概述
====
Dustbin使用jinja2模板系统, 可以在模板中访问模型(Model)获取文章等数据.

目录结构
========
Dustbin的主题必须有一个templates目录, 可以有static目录以储存静态文件

templates/:

<table>
<tr><th>名称</th><th>描述</th></tr>
<tr><td>layout.html</td><td>(**建议**)所有页面的布局</td></tr>
<tr><td>page.html</td><td>(**必须**)主页指定页数的页面</td></tr>
<tr><td>article.html</td><td>(**必须**)显示文章的页面</td></tr>
<tr><td>custom_page.html</td><td>(**必须**)显示自定义页面的页面</td></tr>
<tr><td>archives.html</td><td>(**必须**)显示按年归档的页面</td></tr>
<tr><td>tag.html</td><td>(**必须**)按标签过滤的文章列表页面</td></tr>
<tr><td>404.html</td><td>(**必须**)404页面</td></tr>
</table>

模板
====
变量类型
--------
###Article字典
<table>
<tr><th>项目</th><th>描述</th></tr>
<tr>
    <td>id</td>
    <td>文章ID</td>
</tr>
<tr>
    <td>title</td>
    <td>文章标题</td>
</tr>
<tr>
    <td>content</td>
    <td>文章内容</td>
</tr>
<tr>
    <td>tags</td>
    <td>文章标签列表</td>
</tr>
<tr>
    <td>date</td>
    <td>文章发表日期(UNIX时间戳)</td>
</tr>
</table>

###CustomPage字典
<table>
<tr><th>项目</th><th>描述</th></tr>
<tr>
    <td>id</td>
    <td>页面ID</td>
</tr>
<tr>
    <td>title</td>
    <td>页面标题</td>
</tr>
<tr>
    <td>content</td>
    <td>页面内容</td>
</tr>
<tr>
    <td>order</td>
    <td>页面顺序</td>
</tr>
</table>

###Archive字典
<table>
<tr><th>项目</th><th>描述</th></tr>
<tr>
    <td>articles</td>
    <td>Article字典的列表</td>
</tr>
<tr>
    <td>year</td>
    <td>当前年份</td>
</tr>
</table>

page.html
-----------
<table>
<tr><th>变量</th><th>类型</th><th>描述</th></tr>
<tr>
    <td>page</td>
    <td>int</td>
    <td>当前页面</td>
</tr>
<tr>
    <td>pages</td>
    <td>int</td>
    <td>页面总数</td>
</tr>
<tr>
    <td>has_prev</td>
    <td>bool</td>
    <td>是否有前一页面</td>
</tr>
<tr>
    <td>prev</td>
    <td>int</td>
    <td>前一页面(只有has_prev为true才有此变量)</td>
</tr>
<tr>
    <td>has_next</td>
    <td>bool</td>
    <td>是否有后一页面</td>
</tr>
<tr>
    <td>next</td>
    <td>int</td>
    <td>后一页面(只有has_next为true才有此变量)</td>
</tr>
<tr>
    <td>articles</td>
    <td>list</td>
    <td>当前页面的文章(Article字典的列表)</td>
</tr>
</table>

archives.html
-------------
<table>
<tr><th>变量</th><th>类型</th><th>描述</th></tr>
<tr>
    <td>page</td>
    <td>int</td>
    <td>当前页面</td>
</tr>
<tr>
    <td>pages</td>
    <td>int</td>
    <td>页面总数</td>
</tr>
<tr>
    <td>has_prev</td>
    <td>bool</td>
    <td>是否有前一页面</td>
</tr>
<tr>
    <td>prev</td>
    <td>int</td>
    <td>前一页面(只有has_prev为true才有此变量)</td>
</tr>
<tr>
    <td>has_next</td>
    <td>bool</td>
    <td>是否有后一页面</td>
</tr>
<tr>
    <td>next</td>
    <td>int</td>
    <td>后一页面(只有has_next为true才有此变量)</td>
</tr>
<tr>
    <td>archives</td>
    <td>list</td>
    <td>当前页面的归档(Archive字典的列表)</td>
</tr>
</table>

tag.html
--------
与page.html的变量大致相同, 多了name变量表示标签名字

article.html
------------
<table>
<tr><th>变量</th><th>类型</th><th>描述</th></tr>
<tr>
    <td>article</td>
    <td>Article字典</td>
    <td>要显示的文章</td>
</tr>
</table>

custom_page.html
------------
<table>
<tr><th>变量</th><th>类型</th><th>描述</th></tr>
<tr>
    <td>page</td>
    <td>CustomPage字典</td>
    <td>要显示的自定义页面</td>
</tr>
</table>

**注**:以上所有模板都有custom_pages变量, 提供不包括内容(content为空)的自定义页面.

全局函数
================
url_for_page(page)
------------------
文章列表的URL

page: 页面数

url_for_article(id)
-------------------
文章页面的URL

id: 文章id

url_for_custom_page(id)
-------------------
自定义页面的URL

id: 自定义页面id

url_for_archives(page)
----------------------
存档页面的URL

page: 存档页面的页面数(大于等于1)

url_for_tag(tag, page)
----------------------
标签页面的URL

tag: tag名字

page: 标签页面的页面数(大于等于1)

url_for_static(path)
--------------------
静态文件的URL

path: 静态文件路径

url_for_prev_article(id)
------------------------
前一文章的URL

id: 当前文章ID

url_for_next_article(id)
------------------------
后一文章的URL

id: 当前文章ID

url_for_feed()
--------------
订阅的URL

get_articles(articles_per_page, page, query)
--------------------------------------------
获取多个文章, 返回一个Article字典的列表

articles_per_page: 每页文章数, -1则不分页

page: 页面数(大于等于1)

query: 查询条件, 可以为 `{}` 或 `{'tags':'YOUR_TAG'}`

示例:

```
{%- set articles = get_articles(20, 1, {}) -%}
{% for article in articles %}
    {{ article.title }}
{% endfor %}
```

get_article(id)
---------------
通过ID获取文章, 返回Article字典

id: 文章id

示例:`{{ get_article('hello').title }}`

get_pages()
-----------
获取自定义页面, 返回CustomPage字典的列表

示例:

```
{%- set pages = get_pages() -%}
{% for page in pages %}
    {{ page.title }}
{% endfor %}
```

get_page(id)
------------
通过ID获取自定义页面, 返回CustomPage字典

id: 自定义页面的id

示例: `{{ get_page('hello').title }}`

py_import(name)
---------------
加载一个Python模块

name: 模块名

示例:

```
{%- set math = py_import('math') -%}
{{ math.floor(1.5) }}
```

过滤器
======
format_time(timestamp, format)
-----------
格式化时间

timestamp: 时间戳

format: 格式

示例: `{{ article.date|format_time('%F') }}`