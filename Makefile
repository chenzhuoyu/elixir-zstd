MIX = mix
CFLAGS = -g -O3 -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers

SCRIPT = io:format("~s", [lists:concat([  \
    code:root_dir(),                      \
    "/erts-",                             \
    erlang:system_info(version),          \
    "/include"                            \
])])

ERLANG = $(shell erl -eval '$(SCRIPT)' -s init stop -noshell)
CFLAGS += -I$(ERLANG)

ifeq ($(wildcard deps/libzstd),)
	ZSTD_PATH = ../libzstd
else
	ZSTD_PATH = deps/libzstd
endif

CFLAGS += -I$(ZSTD_PATH)/lib
CFLAGS += -fPIC -flto=full
CFLAGS += -DZSTD_STATIC_LINKING_ONLY

ifeq ($(shell uname),Darwin)
	LDFLAGS += -dynamiclib -undefined dynamic_lookup
endif

.PHONY: all zstd clean

all: zstd

zstd:
	$(MIX) compile

priv/nif_ex_zstd.so: c_src/nif_ex_zstd.c
	$(MAKE) -C $(ZSTD_PATH) lib-release
	$(CC) $(CFLAGS) -shared $(LDFLAGS) -o $@ c_src/nif_ex_zstd.c $(ZSTD_PATH)/lib/libzstd.a

clean:
	$(MIX) clean
	$(MAKE) -C $(ZSTD_PATH) clean
	$(RM) -vrf priv/nif_ex_zstd.so*
