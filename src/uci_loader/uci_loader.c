#include "uci_loader.h"

static int set_option(struct uci_context *ctx, const char *section,
                      const char *option, const char *value) {
  char opt_path[256];

  snprintf(opt_path, sizeof(opt_path), "%s.%s.%s", CONF_FILE, section, option);

  struct uci_ptr ptr = {0};

  if (uci_lookup_ptr(ctx, &ptr, opt_path, false) != UCI_OK) {
    return -1;
  }

  ptr.value = value;

  if (uci_set(ctx, &ptr) != UCI_OK) {
    return -1;
  }

  return 0;
}

int config_load(app_config *cfg) {
  struct uci_context *ctx = uci_alloc_context();
  int ret = 0;
  if (!ctx) {
    ret = -1;
    goto cleanup;
  }

  struct uci_package *pkg = NULL;
  if (uci_load(ctx, CONF_FILE, &pkg) != UCI_OK) {
    ret = -1;
    goto cleanup;
  }

  struct uci_element *e;
  uci_foreach_element(&pkg->sections, e) {
    struct uci_section *s = uci_to_section(e);
    const char *v;

    v = uci_lookup_option_string(ctx, s, "ifname");
    if (v)
      strncpy(cfg->ifname, v, sizeof(cfg->ifname) - 1);

    v = uci_lookup_option_string(ctx, s, "time");
    if (v)
      cfg->interval = strtoll(v, NULL, 10);
  }

cleanup:
  if (ctx)
    uci_unload(ctx, pkg);
  if (pkg)
    uci_free_context(ctx);

  return ret;
}

int config_save(const app_config *cfg) {
  struct uci_context *ctx = uci_alloc_context();
  int ret = 0;

  if (!ctx) {
    ret = -1;
    goto cleanup;
  }

  struct uci_package *pkg = NULL;

  if (uci_load(ctx, CONF_FILE, &pkg) != UCI_OK) {
    ret = -1;
    goto cleanup;
  }

  if (set_option(ctx, "collector", "ifname", cfg->ifname) < 0) {
    ret = -1;
    goto cleanup;
  };

  char time_buf[256];
  snprintf(time_buf, sizeof(time_buf), "%lu", cfg->interval);
  if (set_option(ctx, "collector", "time", time_buf) < 0) {
    ret = -1;
    goto cleanup;
  }

  uci_save(ctx, pkg);
  uci_commit(ctx, &pkg, false);

cleanup:
  if (ctx)
    uci_unload(ctx, pkg);
  if (pkg)
    uci_free_context(ctx);

  return ret;
}
