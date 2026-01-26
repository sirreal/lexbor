/*
 * Minimal reproduction of script tag parsing bug.
 *
 * Bug: Alphabetic characters before the first digit in end tags
 * inside <script> are stripped during parsing.
 *
 * Expected: "</abc123>" preserved as "</abc123>"
 * Actual:   "</abc123>" becomes "</123>"
 */

#include "base.h"

int main(int argc, const char *argv[]) {
  lxb_status_t status;
  lxb_html_document_t *document;

  static const lxb_char_t html[] = "<script>\"</abc123>\"</script>\n"
                                   "<script>\"</abc-123>\"</script>\n"
                                   "<style>/* </abc123> */</style>";
  size_t html_len = sizeof(html) - 1;

  document = lxb_html_document_create();
  if (document == NULL) {
    FAILED("Failed to create HTML Document");
  }

  status = lxb_html_document_parse(document, html, html_len);
  if (status != LXB_STATUS_OK) {
    FAILED("Failed to parse HTML");
  }

  PRINT("** Input: **\n===");
  PRINT("%s", (const char *)html);
  PRINT("===");
  PRINT("** Output: **\n===");
  status = lxb_html_serialize_tree_cb(lxb_dom_interface_node(document),
                                      serializer_callback, NULL);
  if (status != LXB_STATUS_OK) {
    FAILED("Failed to serialize HTML tree");
  }
  PRINT("\n===");
  PRINT("** Expected: **\n===");
  PRINT("<html><head>"
        "<script>\"</abc123>\"</script>\n"
        "<script>\"</abc-123>\"</script>\n"
        "<style>/* </abc123> */</style>"
        "</head><body></body></html>");
  PRINT("===");

  lxb_html_document_destroy(document);

  return 0;
}
