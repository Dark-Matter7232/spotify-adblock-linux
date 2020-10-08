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
static typeof(curl_easy_setopt) *real_setopt = NULL;
static typeof(cef_urlrequest_create) *real_cef_urlrequest_create = NULL;

static bool verbose = false;

static void init_real_getaddrinfo(void) {
    real_getaddrinfo = dlsym(RTLD_NEXT, "getaddrinfo");
    if (!real_getaddrinfo) {
        LOG_RED("dlsym (getaddrinfo): %s", dlerror());
    }
}

static void init_real_setopt(void) {
    real_setopt = dlsym(RTLD_NEXT, "curl_easy_setopt");
    if (!real_setopt) {
        LOG_RED("dlsym (curl_easy_setopt): %s", dlerror());
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
        if (!fnmatch(whitelist[i], node, 0)) {
            if (verbose) LOG_GREEN("[+] getaddrinfo:\t\t%s", node);
            if (!real_getaddrinfo) init_real_getaddrinfo();
            return real_getaddrinfo(node, service, hints, res);
        }
    }
    if (verbose) LOG_RED("[-] getaddrinfo:\t\t%s", node);
    return EAI_FAIL;
}

CURLcode curl_easy_setopt2(CURL *handle, CURLoption option, ...) {
    if (option == CURLOPT_URL) {
        va_list args;
        va_start(args, option);
        char *url = va_arg(args, char *);
        va_end(args);
        for (int i = 0; i < sizeof(blacklist) / sizeof(blacklist[0]); i++) {
            if (!fnmatch(blacklist[i], url, 0)) {
                if (verbose) LOG_RED("[-] curl_easy_setopt:\t\t%s\n", url);
                // destroy handle, so the request can never be attempted
                curl_easy_cleanup(handle);
                handle = NULL;
                return CURLE_OK;
            }
        }
        if (verbose) LOG_GREEN("[+] curl_easy_setopt:\t\t%s\n", url);
    }
    if (!real_setopt) init_real_setopt();
    void *args = __builtin_apply_args();
    void *ret = __builtin_apply((void *)real_setopt, args, 500);
    __builtin_return(ret);
}

__attribute__((constructor)) static void injectedConstructor(
    int argc, const char **argv) {
    char *verbose_var = getenv("SPOTIFY_ADBLOCK_VERBOSE");
    if (verbose_var != NULL) {
        printf("Successfully injected\n");
        verbose = true;
    }
}
cef_urlrequest_t* cef_urlrequest_create(struct _cef_request_t* request, struct _cef_urlrequest_client_t* client, struct _cef_request_context_t* request_context) {
    cef_string_userfree_utf16_t url_utf16 = request->get_url(request);
    char url[url_utf16->length + 1];
    url[url_utf16->length] = '\0';
    for (int i = 0; i < url_utf16->length; i++) url[i] = *(url_utf16->str + i);
    cef_string_userfree_utf16_free(url_utf16);
    for (int i = 0; i < sizeof(blacklist) / sizeof(blacklist[0]); i++) {
        if (!fnmatch(blacklist[i], url, 0)) {
            printf("[-] cef_urlrequest_create:\t%s", url);
            return NULL;
        }
    }
    printf("[+] cef_urlrequest_create:\t%s", url);
    return real_cef_urlrequest_create(request, client, request_context);
}
// Interpose our overrides
DYLD_INTERPOSE(getaddrinfo2, getaddrinfo)
DYLD_INTERPOSE(curl_easy_setopt2, curl_easy_setopt)
