����
====
Dustbinʹ��jinja2ģ��ϵͳ, ������ģ���з���ģ��(Model)��ȡ���µ�����.

Ŀ¼�ṹ
========
Dustbin�����������һ��templatesĿ¼, ������staticĿ¼�Դ��澲̬�ļ�

templates/:

<table>
<tr><th>����</th><th>����</th></tr>
<tr><td>layout.html</td><td>(**����**)����ҳ��Ĳ���</td></tr>
<tr><td>page.html</td><td>(**����**)��ҳָ��ҳ����ҳ��</td></tr>
<tr><td>article.html</td><td>(**����**)��ʾ���µ�ҳ��</td></tr>
<tr><td>custom_page.html</td><td>(**����**)��ʾ�Զ���ҳ���ҳ��</td></tr>
<tr><td>archives.html</td><td>(**����**)��ʾ����鵵��ҳ��</td></tr>
<tr><td>tag.html</td><td>(**����**)����ǩ���˵������б�ҳ��</td></tr>
<tr><td>404.html</td><td>(**����**)404ҳ��</td></tr>
</table>

ģ��
====
��������
--------
###Article�ֵ�
<table>
<tr><th>��Ŀ</th><th>����</th></tr>
<tr>
    <td>id</td>
    <td>����ID</td>
</tr>
<tr>
    <td>title</td>
    <td>���±���</td>
</tr>
<tr>
    <td>content</td>
    <td>��������</td>
</tr>
<tr>
    <td>tags</td>
    <td>���±�ǩ�б�</td>
</tr>
<tr>
    <td>date</td>
    <td>���·�������(UNIXʱ���)</td>
</tr>
</table>

###CustomPage�ֵ�
<table>
<tr><th>��Ŀ</th><th>����</th></tr>
<tr>
    <td>id</td>
    <td>ҳ��ID</td>
</tr>
<tr>
    <td>title</td>
    <td>ҳ�����</td>
</tr>
<tr>
    <td>content</td>
    <td>ҳ������</td>
</tr>
<tr>
    <td>order</td>
    <td>ҳ��˳��</td>
</tr>
</table>

###Archive�ֵ�
<table>
<tr><th>��Ŀ</th><th>����</th></tr>
<tr>
    <td>articles</td>
    <td>Article�ֵ���б�</td>
</tr>
<tr>
    <td>year</td>
    <td>��ǰ���</td>
</tr>
</table>

page.html
-----------
<table>
<tr><th>����</th><th>����</th><th>����</th></tr>
<tr>
    <td>page</td>
    <td>int</td>
    <td>��ǰҳ��</td>
</tr>
<tr>
    <td>pages</td>
    <td>int</td>
    <td>ҳ������</td>
</tr>
<tr>
    <td>has_prev</td>
    <td>bool</td>
    <td>�Ƿ���ǰһҳ��</td>
</tr>
<tr>
    <td>prev</td>
    <td>int</td>
    <td>ǰһҳ��(ֻ��has_prevΪtrue���д˱���)</td>
</tr>
<tr>
    <td>has_next</td>
    <td>bool</td>
    <td>�Ƿ��к�һҳ��</td>
</tr>
<tr>
    <td>next</td>
    <td>int</td>
    <td>��һҳ��(ֻ��has_nextΪtrue���д˱���)</td>
</tr>
<tr>
    <td>articles</td>
    <td>list</td>
    <td>��ǰҳ�������(Article�ֵ���б�)</td>
</tr>
</table>

archives.html
-------------
<table>
<tr><th>����</th><th>����</th><th>����</th></tr>
<tr>
    <td>page</td>
    <td>int</td>
    <td>��ǰҳ��</td>
</tr>
<tr>
    <td>pages</td>
    <td>int</td>
    <td>ҳ������</td>
</tr>
<tr>
    <td>has_prev</td>
    <td>bool</td>
    <td>�Ƿ���ǰһҳ��</td>
</tr>
<tr>
    <td>prev</td>
    <td>int</td>
    <td>ǰһҳ��(ֻ��has_prevΪtrue���д˱���)</td>
</tr>
<tr>
    <td>has_next</td>
    <td>bool</td>
    <td>�Ƿ��к�һҳ��</td>
</tr>
<tr>
    <td>next</td>
    <td>int</td>
    <td>��һҳ��(ֻ��has_nextΪtrue���д˱���)</td>
</tr>
<tr>
    <td>archives</td>
    <td>list</td>
    <td>��ǰҳ��Ĺ鵵(Archive�ֵ���б�)</td>
</tr>
</table>

tag.html
--------
��page.html�ı���������ͬ, ����name������ʾ��ǩ����

article.html
------------
<table>
<tr><th>����</th><th>����</th><th>����</th></tr>
<tr>
    <td>article</td>
    <td>Article�ֵ�</td>
    <td>Ҫ��ʾ������</td>
</tr>
</table>

custom_page.html
------------
<table>
<tr><th>����</th><th>����</th><th>����</th></tr>
<tr>
    <td>page</td>
    <td>CustomPage�ֵ�</td>
    <td>Ҫ��ʾ���Զ���ҳ��</td>
</tr>
</table>

**ע**:��������ģ�嶼��custom_pages����, �ṩ����������(contentΪ��)���Զ���ҳ��.

ȫ�ֺ���
================
url_for_page(page)
------------------
�����б��URL

page: ҳ����

url_for_article(id)
-------------------
����ҳ���URL

id: ����id

url_for_custom_page(id)
-------------------
�Զ���ҳ���URL

id: �Զ���ҳ��id

url_for_archives(page)
----------------------
�浵ҳ���URL

page: �浵ҳ���ҳ����(���ڵ���1)

url_for_tag(tag, page)
----------------------
��ǩҳ���URL

tag: tag����

page: ��ǩҳ���ҳ����(���ڵ���1)

url_for_static(path)
--------------------
��̬�ļ���URL

path: ��̬�ļ�·��

url_for_prev_article(id)
------------------------
ǰһ���µ�URL

id: ��ǰ����ID

url_for_next_article(id)
------------------------
��һ���µ�URL

id: ��ǰ����ID

url_for_feed()
--------------
���ĵ�URL

get_articles(articles_per_page, page, query)
--------------------------------------------
��ȡ�������, ����һ��Article�ֵ���б�

articles_per_page: ÿҳ������, -1�򲻷�ҳ

page: ҳ����(���ڵ���1)

query: ��ѯ����, ����Ϊ `{}` �� `{'tags':'YOUR_TAG'}`

ʾ��:

```
{%- set articles = get_articles(20, 1, {}) -%}
{% for article in articles %}
    {{ article.title }}
{% endfor %}
```

get_article(id)
---------------
ͨ��ID��ȡ����, ����Article�ֵ�

id: ����id

ʾ��:`{{ get_article('hello').title }}`

get_pages()
-----------
��ȡ�Զ���ҳ��, ����CustomPage�ֵ���б�

ʾ��:

```
{%- set pages = get_pages() -%}
{% for page in pages %}
    {{ page.title }}
{% endfor %}
```

get_page(id)
------------
ͨ��ID��ȡ�Զ���ҳ��, ����CustomPage�ֵ�

id: �Զ���ҳ���id

ʾ��: `{{ get_page('hello').title }}`

py_import(name)
---------------
����һ��Pythonģ��

name: ģ����

ʾ��:

```
{%- set math = py_import('math') -%}
{{ math.floor(1.5) }}
```

������
======
format_time(timestamp, format)
-----------
��ʽ��ʱ��

timestamp: ʱ���

format: ��ʽ

ʾ��: `{{ article.date|format_time('%F') }}`