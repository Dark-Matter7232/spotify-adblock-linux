#define _GNU_SOURCE

#include <curl/curl.h>
#include <dlfcn.h>
#include <fnmatch.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "include/capi/cef_urlrequest_capi.h"
#include "blacklist.h"
#include "whitelist.h"
#include <stdbool.h>

#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_RESET "\x1b[0m"

#define LOG_RED(template, ...) \
    printf("%s" template "%s\n", ANSI_RED, ##__VA_ARGS__, ANSI_RESET)

#define LOG_GREEN(template, ...) \
    printf("%s" template "%s\n", ANSI_GREEN, ##__VA_ARGS__, ANSI_RESET)

#define DYLD_INTERPOSE(_replacement, _replacee)            \
    __attribute__((used)) static struct {                  \
        const void *replacement;                           \
        const void *replacee;                              \
    } _interpose_##_replacee                               \
        __attribute__((section("__DATA,__interpose"))) = { \
            (const void *)(unsigned long)&_replacement,    \
            (const void *)(unsigned long)&_replacee};

static typeof(getaddrinfo) *real_getaddrinfo = NULL;
static typeof(cef_urlrequest_create) *real_cef_urlrequest_create = NULL;
const ssize_t whitelist_size = sizeof(whitelist) / sizeof(whitelist[0]);
const ssize_t blacklist_size = sizeof(blacklist) / sizeof(blacklist[0]);
static bool verbose = false;
bool listed(const char *item, const char *list[], ssize_t list_size) {
    for (ssize_t i = 0; i < list_size; i++) {
        if (!fnmatch(list[i], item, 0)) {
            return true;
        }
    }
    return false;
}

static void init_real_getaddrinfo(void) {
    real_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
    if (!real_getaddrinfo) {
        LOG_RED("dlsym (getaddrinfo): %s", dlerror());
    }
}

int getaddrinfo2(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res) {
    // This might break some things?
    if (node == NULL) {
        if (verbose) LOG_RED("[-] NULL");
        return EAI_FAIL;
    }
    for (int i = 0; i < sizeof(whitelist) / sizeof(whitelist[0]); i++) {
        if (!fnmatch(whitelist[i], node, FNM_NOESCAPE)) {
            if (verbose) LOG_GREEN("[+] %s", node);
            if (!real_getaddrinfo) init_real_getaddrinfo();
            return real_getaddrinfo(node, service, hints, res);
        }
    }
    if (verbose) LOG_RED("[-] %s", node);
    return EAI_FAIL;
}

cef_urlrequest_t* cef_urlrequest_create2(struct _cef_request_t* request, struct _cef_urlrequest_client_t* client, struct _cef_request_context_t* request_context) {
    cef_string_userfree_utf16_t url_utf16 = request->get_url(request);
    char url[url_utf16->length + 1];
    url[url_utf16->length] = '\0';
    for (int i = 0; i < url_utf16->length; i++) url[i] = *(url_utf16->str + i);
    cef_string_userfree_utf16_free(url_utf16);
    if (listed(url, blacklist, blacklist_size)) {
        LOG_GREEN("[-] cef_urlrequest_create2:\t%s", url);
        return NULL;
    }
    LOG_GREEN("[+] cef_urlrequest_create2:\t%s", url);
    return real_cef_urlrequest_create(request, client, request_context);
}
__attribute__((constructor)) static void injectedConstructor(
    int argc, const char **argv) {
    char *verbose_var = getenv("SPOTIFY_ADBLOCK_VERBOSE");
    if (verbose_var != NULL) {
        printf("Successfully injected\n");
        verbose = true;
    }
}

// Interpose our overrides
DYLD_INTERPOSE(getaddrinfo2, getaddrinfo)
DYLD_INTERPOSE(cef_urlrequest_create2, cef_urlrequest_create)