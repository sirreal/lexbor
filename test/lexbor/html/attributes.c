/*
 * Copyright (C) 2021 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#include <lexbor/html/html.h>
#include <lexbor/dom/dom.h>

#include <unit/test.h>


const lxb_char_t html[] = "<div id=my-best-id></div>";

const size_t html_length = sizeof(html) - 1;

const lxb_char_t foreign_attrs_html[] = "<svg xlink:href=qual href=plain>";

const size_t foreign_attrs_html_length = sizeof(foreign_attrs_html) - 1;


TEST_BEGIN(attrs)
{
    size_t i, size;
    lxb_status_t status;
    lxb_html_body_element_t *body;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;
    lxb_dom_element_t *element;
    lxb_dom_attr_t *attr;

    bool is_exist;
    const lxb_char_t *name;
    const lxb_char_t *value;
    size_t value_len;

    static const char *names[] = {"id", "class", "some", NULL};

    /* Parse. */
    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, html, html_length);
    test_eq(status, LXB_STATUS_OK);

    /* Collection for elements. */
    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    /* Get BODY element (root for search) */
    body = lxb_html_document_body_element(document);
    element = lxb_dom_interface_element(body);

    /* Find DIV eleemnt */
    status = lxb_dom_elements_by_tag_name(element, collection,
                                          (const lxb_char_t *) "div", 3);

    test_eq(status, LXB_STATUS_OK);
    test_ne(lxb_dom_collection_length(collection), 0);

    /* Append new attribute */
    element = lxb_dom_collection_element(collection, 0);

    i = 0;

    while (names[i] != NULL) {
        name = (const lxb_char_t *) names[i++];
        size = strlen((const char *) name);

        attr = lxb_dom_element_set_attribute(element, name, size,
                                             (const lxb_char_t *) "oh God", 6);
        test_ne(attr, NULL);

        /* Check exist */
        is_exist = lxb_dom_element_has_attribute(element, name, size);
        test_eq(is_exist, true);

        /* Get value by qualified name */
        value = lxb_dom_element_get_attribute(element, name,
                                              size, &value_len);
        test_ne(value, NULL);

        /* Change value */
        attr = lxb_dom_element_attr_by_name(element, name, size);
        status = lxb_dom_attr_set_value(attr,
                                        (const lxb_char_t *) "new value", 9);

        test_eq(status, LXB_STATUS_OK);

        /* Remove new attribute by name */
        lxb_dom_element_remove_attribute(element, name, size);
    }

    /* Destroy all */
    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

TEST_BEGIN(svg_xlink_href_and_href_attrs)
{
    lxb_status_t status;
    lxb_html_body_element_t *body;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;
    lxb_dom_element_t *element;
    lxb_dom_attr_t *attr, *xlink_attr, *href_attr;
    const lxb_char_t *name;
    const lxb_char_t *value;
    size_t len;
    size_t count;

    document = lxb_html_document_create();
    test_ne(document, NULL);

    status = lxb_html_document_parse(document, foreign_attrs_html,
                                     foreign_attrs_html_length);
    test_eq(status, LXB_STATUS_OK);

    collection = lxb_dom_collection_make(&document->dom_document, 16);
    test_ne(collection, NULL);

    body = lxb_html_document_body_element(document);
    element = lxb_dom_interface_element(body);

    status = lxb_dom_elements_by_tag_name(element, collection,
                                          (const lxb_char_t *) "svg", 3);

    test_eq(status, LXB_STATUS_OK);
    test_eq(lxb_dom_collection_length(collection), 1);

    element = lxb_dom_collection_element(collection, 0);

    count = 0;
    xlink_attr = NULL;
    href_attr = NULL;
    attr = lxb_dom_element_first_attribute(element);

    while (attr != NULL) {
        count++;

        name = lxb_dom_attr_qualified_name(attr, &len);

        if (len == 10
            && lexbor_str_data_ncmp(name, (const lxb_char_t *) "xlink:href",
                                    len))
        {
            xlink_attr = attr;
        }
        else if (len == 4
                 && lexbor_str_data_ncmp(name, (const lxb_char_t *) "href",
                                         len))
        {
            href_attr = attr;
        }

        attr = lxb_dom_element_next_attribute(attr);
    }

    test_eq(count, 2);
    test_ne(xlink_attr, NULL);
    test_ne(href_attr, NULL);
    test_ne(xlink_attr, href_attr);

    test_eq(xlink_attr->node.ns, LXB_NS_XLINK);
    test_eq(href_attr->node.ns, LXB_NS_SVG);

    name = lxb_dom_attr_local_name(xlink_attr, &len);
    test_eq_str_n(name, len, (const lxb_char_t *) "href", 4);

    name = lxb_dom_attr_local_name(href_attr, &len);
    test_eq_str_n(name, len, (const lxb_char_t *) "href", 4);

    value = lxb_dom_attr_value(xlink_attr, &len);
    test_eq_str_n(value, len, (const lxb_char_t *) "qual", 4);

    value = lxb_dom_attr_value(href_attr, &len);
    test_eq_str_n(value, len, (const lxb_char_t *) "plain", 5);

    lxb_dom_collection_destroy(collection, true);
    lxb_html_document_destroy(document);
}
TEST_END

int
main(int argc, const char * argv[])
{
    TEST_INIT();

    TEST_ADD(attrs);
    TEST_ADD(svg_xlink_href_and_href_attrs);

    TEST_RUN("lexbor/html/attributes");
    TEST_RELEASE();
}
