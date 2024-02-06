#include "../src/json.h"
#include "../src/logger.h"
#include "../src/string_utils.h"
#include "../src/uri.h"

typedef struct {
  size_t n;
} Server;

typedef struct {
  UriComponents *components;
  StringView uri;
} Context;

typedef struct {
  StringView path;
  void (*handler)(Context *ctx);
} Route;

#define MAX_ROUTES 10

typedef struct {
  Route routes[MAX_ROUTES];
  size_t count;
} Router;

void handler_admin(Context *ctx) {
  printf("\nAdmin handler called for URI: ");
  sv_print(&ctx->uri);
}

void handler_admin_history(Context *ctx) {
  printf("\nAdmin handler called for URI: ");
  printf("Admin history: ...");
}

void handler_user(Context *ctx) {
  printf("\nUser handler called for URI: ");
  sv_print(&ctx->uri);
}

void handler_user_history(Context *ctx) {
  printf("\nUser handler called for URI: ");
  printf("User history: ...");
}

// Add a route to the router
void add_route(Router *router, StringView path, void (*handler)(Context *)) {
  if (router->count < MAX_ROUTES) {
    router->routes[router->count++] = (Route){.path = path, .handler = handler};
  } else {
    logger_log(LOG_ERROR, "Maximum number of routes reached");
  }
}

void test_fake_server() {
  // Sample fake data
  StringView paths[4] = {
      SV_NEW_FROM_CSTR("https://www.mojastrona.com/admin"),
      SV_NEW_FROM_CSTR("https://www.mojastrona.com/admin/history"),
      SV_NEW_FROM_CSTR("https://www.mojastrona.com/user"),
      SV_NEW_FROM_CSTR("https://www.mojastrona.com/user/history"),
  };

  StringView bodies[4] = {
      SV_NEW_FROM_CSTR("{\"name\": \"Kamil\", \"age\": 24}"),
      SV_NEW_FROM_CSTR("{\"name\": \"Kamil\", \"age\": 24}"),
      SV_NEW_FROM_CSTR("{\"name\": \"Ktos\", \"age\": 16}"),
      SV_NEW_FROM_CSTR("{\"name\": \"Ktos\", \"age\": 16}"),
  };

  Router router = {0};
  add_route(&router, SV_NEW_FROM_CSTR("admin"), handler_admin);
  add_route(&router, SV_NEW_FROM_CSTR("admin/history"), handler_admin_history);
  add_route(&router, SV_NEW_FROM_CSTR("user"), handler_user);
  add_route(&router, SV_NEW_FROM_CSTR("user/history"), handler_user_history);

  for (size_t i = 0; i < 4; ++i) {
    UriComponents components = {0};
    uri_parse(paths[i], &components);

    Json *body = json_new();
    if (!json_parse(&bodies[i], body)) {
      json_free(body);
      continue;
    }

    StringView first_param = sv_pop_first_split_by(&components.path, SV_NEW_FROM_CSTR("/"));

    Route *matched_route = NULL;
    for (size_t j = 0; j < router.count; ++j) {
      if (sv_compare(router.routes[j].path, first_param)) {
        matched_route = &router.routes[j];
        break;
      }
    }

    if (matched_route != NULL) {
      Context ctx = {.uri = paths[i], .components = &components};
      matched_route->handler(&ctx);
    } else {
      logger_log(LOG_INFO, "unreachable path");
    }
  }
}
