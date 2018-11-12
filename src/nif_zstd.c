#include <zstd.h>
#include <erl_nif.h>

static ERL_NIF_TERM _zstd_cstream_new(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM _zstd_dstream_new(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM _zstd_cstream_init(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM _zstd_dstream_init(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM _zstd_cstream_reset(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM _zstd_dstream_reset(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM _zstd_stream_flush(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM _zstd_stream_compress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM _zstd_stream_decompress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM _zstd_simple_compress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM _zstd_simple_decompress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[]);

static ErlNifFunc _zstd_exports[] = {
    { "cstream_new"      , 0, _zstd_cstream_new      },
    { "dstream_new"      , 0, _zstd_dstream_new      },

    { "cstream_init"     , 1, _zstd_cstream_init     , ERL_DIRTY_JOB_CPU_BOUND },
    { "cstream_init"     , 2, _zstd_cstream_init     , ERL_DIRTY_JOB_CPU_BOUND },
    { "dstream_init"     , 1, _zstd_dstream_init     , ERL_DIRTY_JOB_CPU_BOUND },

    { "cstream_reset"    , 2, _zstd_cstream_reset    },
    { "cstream_reset"    , 1, _zstd_cstream_reset    },
    { "dstream_reset"    , 1, _zstd_dstream_reset    },

    { "stream_flush"     , 1, _zstd_stream_flush     , ERL_DIRTY_JOB_CPU_BOUND },
    { "stream_compress"  , 2, _zstd_stream_compress  , ERL_DIRTY_JOB_CPU_BOUND },
    { "stream_decompress", 2, _zstd_stream_decompress, ERL_DIRTY_JOB_CPU_BOUND },

    { "simple_compress"  , 1, _zstd_simple_compress  , ERL_DIRTY_JOB_CPU_BOUND },
    { "simple_compress"  , 2, _zstd_simple_compress  , ERL_DIRTY_JOB_CPU_BOUND },
    { "simple_decompress", 1, _zstd_simple_decompress, ERL_DIRTY_JOB_CPU_BOUND },
};

static ERL_NIF_TERM _atom_ok;
static ERL_NIF_TERM _atom_error;
static ERL_NIF_TERM _atom_einval;
static ERL_NIF_TERM _atom_enomem;

static ErlNifResourceType *_zstd_cstream_type = NULL;
static ErlNifResourceType *_zstd_dstream_type = NULL;

static inline ERL_NIF_TERM _zstd_wrap_inval(ErlNifEnv *self)
{
    return enif_make_tuple2(
        self,
        _atom_error,
        _atom_einval
    );
}

static inline ERL_NIF_TERM _zstd_wrap_nomem(ErlNifEnv *self)
{
    return enif_make_tuple2(
        self,
        _atom_error,
        _atom_enomem
    );
}

static inline ERL_NIF_TERM _zstd_wrap_error(ErlNifEnv *self, size_t ret)
{
    return enif_make_tuple2(
        self,
        _atom_error,
        enif_make_string(
            self,
            ZSTD_getErrorName(ret),
            ERL_NIF_LATIN1
        )
    );
}

static inline ERL_NIF_TERM _zstd_wrap_string(ErlNifEnv *self, const char *msg)
{
    return enif_make_tuple2(
        self,
        _atom_error,
        enif_make_string(self, msg, ERL_NIF_LATIN1)
    );
}

static inline ERL_NIF_TERM _zstd_wrap_pointer(ErlNifEnv *self, void *ptr)
{
    ERL_NIF_TERM res = enif_make_resource(self, ptr);
    enif_release_resource(ptr);
    return res;
}

static void _zstd_cstream_dtor(ErlNifEnv *self, void *stream)
{
    ZSTD_CStream **handle = stream;
    ZSTD_freeCStream(*handle);
}

static void _zstd_dstream_dtor(ErlNifEnv *self, void *stream)
{
    ZSTD_DStream **handle = stream;
    ZSTD_freeDStream(*handle);
}

static int _zstd_init(ErlNifEnv *self)
{
    /* cstream type */
    _zstd_cstream_type = enif_open_resource_type(
        self,
        "Elixir.Zstd",
        "CompressStream",
        _zstd_cstream_dtor,
        ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER,
        NULL
    );

    /* cstream type */
    _zstd_dstream_type = enif_open_resource_type(
        self,
        "Elixir.Zstd",
        "DecompressStream",
        _zstd_dstream_dtor,
        ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER,
        NULL
    );

    /* create atoms */
    enif_make_existing_atom(self, "ok"    , &_atom_ok    , ERL_NIF_LATIN1);
    enif_make_existing_atom(self, "error" , &_atom_error , ERL_NIF_LATIN1);
    enif_make_existing_atom(self, "einval", &_atom_einval, ERL_NIF_LATIN1);
    enif_make_existing_atom(self, "enomem", &_atom_enomem, ERL_NIF_LATIN1);

    /* should all be loaded */
    return !(_zstd_cstream_type && _zstd_dstream_type);
}

static int _zstd_on_load(ErlNifEnv *self, void **priv, ERL_NIF_TERM info)                { return _zstd_init(self); }
static int _zstd_on_reload(ErlNifEnv *self, void **priv, ERL_NIF_TERM info)              { return _zstd_init(self); }
static int _zstd_on_upgrade(ErlNifEnv *self, void **priv, void **old, ERL_NIF_TERM info) { return _zstd_init(self); }

static ERL_NIF_TERM _zstd_cstream_new(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    /* create handle */
    ZSTD_CStream **handle = enif_alloc_resource(
        _zstd_cstream_type,
        sizeof(ZSTD_CStream *)
    );

    /* create cstream stream */
    *handle = ZSTD_createCStream();
    return _zstd_wrap_pointer(self, handle);
}

static ERL_NIF_TERM _zstd_dstream_new(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    /* create handle */
    ZSTD_DStream **handle = enif_alloc_resource(
        _zstd_dstream_type,
        sizeof(ZSTD_DStream *)
    );

    /* create decstream stream */
    *handle = ZSTD_createDStream();
    return _zstd_wrap_pointer(self, handle);
}

static ERL_NIF_TERM _zstd_cstream_init(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    int level = ZSTD_CLEVEL_DEFAULT;
    size_t ret;
    ZSTD_CStream **pzcs;

    /* extract the stream */
    if (!(enif_get_resource(self, argv[0], _zstd_cstream_type, (void **)&pzcs)))
        return _zstd_wrap_inval(self);

    /* extract the compression level if any */
    if ((argc == 2) && !(enif_get_int(self, argv[1], &level)))
        return _zstd_wrap_inval(self);

    /* initialize the stream */
    if (ZSTD_isError(ret = ZSTD_initCStream(*pzcs, level)))
        return _zstd_wrap_error(self, ret);

    /* stream initialization successful */
    return _atom_ok;
}

static ERL_NIF_TERM _zstd_dstream_init(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    size_t ret;
    ZSTD_DStream **pzds;

    /* extract the stream */
    if (!(enif_get_resource(self, argv[0], _zstd_dstream_type, (void **)&pzds)))
        return _zstd_wrap_inval(self);

    /* initialize the stream */
    if (ZSTD_isError(ret = ZSTD_initDStream(*pzds)))
        return _zstd_wrap_error(self, ret);

    /* stream initialization successful */
    return _atom_ok;
}

static ERL_NIF_TERM _zstd_cstream_reset(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    size_t ret;
    size_t size = ZSTD_CONTENTSIZE_UNKNOWN;
    ZSTD_CStream **pzcs;

    /* extract the stream */
    if (!(enif_get_resource(self, argv[0], _zstd_cstream_type, (void **)&pzcs)))
        return _zstd_wrap_inval(self);

    /* extract the pledged source size if any */
    if ((argc == 2) && !(enif_get_ulong(self, argv[1], &size)))
        return _zstd_wrap_inval(self);

    /* reset the stream */
    if (ZSTD_isError(ret = ZSTD_resetCStream(*pzcs, size)))
        return _zstd_wrap_error(self, ret);

    /* stream resetting successful */
    return _atom_ok;
}

static ERL_NIF_TERM _zstd_dstream_reset(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    size_t ret;
    ZSTD_DStream **pzds;

    /* extract the stream */
    if (!(enif_get_resource(self, argv[0], _zstd_dstream_type, (void **)&pzds)))
        return _zstd_wrap_inval(self);

    /* reset the stream */
    if (ZSTD_isError(ret = ZSTD_resetDStream(*pzds)))
        return _zstd_wrap_error(self, ret);

    /* stream resetting successful */
    return _atom_ok;
}

static ERL_NIF_TERM _zstd_stream_flush(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    size_t ret;
    ErlNifBinary bin;
    ZSTD_CStream **pzcs;

    /* extract the stream */
    if (!(enif_get_resource(self, argv[0], _zstd_cstream_type, (void **)&pzcs)))
        return _zstd_wrap_inval(self);

    /* allocate binary buffer */
    if (!(enif_alloc_binary(ZSTD_CStreamOutSize(), &bin)))
        return _zstd_wrap_nomem(self);

    /* output buffer */
    ZSTD_outBuffer outbuf = {
        .pos = 0,
        .dst = bin.data,
        .size = bin.size,
    };

    /* reset the stream */
    if (ZSTD_isError(ret = ZSTD_endStream(*pzcs, &outbuf)))
    {
        enif_release_binary(&bin);
        return _zstd_wrap_error(self, ret);
    }

    /* transfer to binary object */
    ERL_NIF_TERM binary = enif_make_binary(self, &bin);
    ERL_NIF_TERM result = binary;

    /* remove unused spaces */
    if (outbuf.pos < outbuf.size)
        result = enif_make_sub_binary(self, binary, 0, outbuf.pos);

    /* construct the result tuple */
    return enif_make_tuple2(self, _atom_ok, result);
}

static ERL_NIF_TERM _zstd_stream_compress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    size_t ret;
    ErlNifBinary in;
    ErlNifBinary out;
    ZSTD_CStream **pzcs;

    /* extract the stream */
    if (!(enif_get_resource(self, argv[0], _zstd_cstream_type, (void **)&pzcs)) ||
        !(enif_inspect_iolist_as_binary(self, argv[1], &in)))
        return _zstd_wrap_inval(self);

    /* all output binary buffer */
    if (!(enif_alloc_binary(ZSTD_compressBound(in.size), &out)))
    {
        enif_release_binary(&in);
        return _zstd_wrap_nomem(self);
    }

    /* input buffer */
    ZSTD_inBuffer inbuf = {
        .pos = 0,
        .src = in.data,
        .size = in.size,
    };

    /* output buffer */
    ZSTD_outBuffer outbuf = {
        .pos = 0,
        .dst = out.data,
        .size = out.size,
    };

    /* compress every chunk */
    while (inbuf.pos < inbuf.size)
    {
        if (ZSTD_isError(ret = ZSTD_compressStream(*pzcs, &outbuf, &inbuf)))
        {
            enif_release_binary(&in);
            enif_release_binary(&out);
            return _zstd_wrap_error(self, ret);
        }
    }

    /* transfer to binary object */
    ERL_NIF_TERM binary = enif_make_binary(self, &out);
    ERL_NIF_TERM result = binary;

    /* remove unused spaces */
    if (outbuf.pos < outbuf.size)
        result = enif_make_sub_binary(self, binary, 0, outbuf.pos);

    /* construct the result tuple */
    enif_release_binary(&in);
    return enif_make_tuple2(self, _atom_ok, result);
}

static ERL_NIF_TERM _zstd_stream_decompress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    size_t ret;
    ErlNifBinary in;
    ErlNifBinary out;
    ZSTD_DStream **pzds;

    /* extract the stream */
    if (!(enif_get_resource(self, argv[0], _zstd_dstream_type, (void **)&pzds)) ||
        !(enif_inspect_iolist_as_binary(self, argv[1], &in)))
        return _zstd_wrap_inval(self);

    /* allocate output binary buffer */
    if (!(enif_alloc_binary(ZSTD_DStreamOutSize(), &out)))
    {
        enif_release_binary(&in);
        return _zstd_wrap_nomem(self);
    }

    /* input buffer */
    ZSTD_inBuffer inbuf = {
        .pos = 0,
        .src = in.data,
        .size = in.size,
    };

    /* output buffer */
    ZSTD_outBuffer outbuf = {
        .pos = 0,
        .dst = out.data,
        .size = out.size,
    };

    /* decompress every chunk */
    while (inbuf.pos < inbuf.size)
    {
        /* enlarge output buffer as needed */
        if (outbuf.size - outbuf.pos < ZSTD_DStreamOutSize())
        {
            /* resize the output binary */
            if (!(enif_realloc_binary(&out, outbuf.size * 2)))
            {
                enif_release_binary(&in);
                enif_release_binary(&out);
                return _zstd_wrap_nomem(self);
            }

            /* update buffer pointers */
            outbuf.dst = out.data + outbuf.pos;
            outbuf.size = out.size;
        }
        fprintf(stderr, "pos: %zu/%zu, %zu/%zu, %d %d %d %d\r\n",
            outbuf.pos, outbuf.size, inbuf.pos, inbuf.size,
            ((const unsigned char *)inbuf.src)[0],
            ((const unsigned char *)inbuf.src)[1],
            ((const unsigned char *)inbuf.src)[2],
            ((const unsigned char *)inbuf.src)[3]
        );

        /* decompress one frame */
        if (ZSTD_isError(ret = ZSTD_decompressStream(*pzds, &outbuf, &inbuf)))
        {
            enif_release_binary(&in);
            enif_release_binary(&out);
            return _zstd_wrap_error(self, ret);
        }
    }

    /* transfer to binary object */
    ERL_NIF_TERM binary = enif_make_binary(self, &out);
    ERL_NIF_TERM result = binary;

    /* remove unused spaces */
    if (outbuf.pos < outbuf.size)
        result = enif_make_sub_binary(self, binary, 0, outbuf.pos);

    /* construct the result tuple */
    enif_release_binary(&in);
    return enif_make_tuple2(self, _atom_ok, result);
}

static ERL_NIF_TERM _zstd_simple_compress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    int level = ZSTD_CLEVEL_DEFAULT;
    size_t ret;
    ErlNifBinary in;
    ErlNifBinary out;

    /* extract the binary data */
    if (!(enif_inspect_iolist_as_binary(self, argv[0], &in)))
        return _zstd_wrap_inval(self);

    /* extract the compression level if any */
    if ((argc == 2) && !(enif_get_int(self, argv[1], &level)))
    {
        enif_release_binary(&in);
        return _zstd_wrap_inval(self);
    }

    /* allocate output binary */
    if (!(enif_alloc_binary(ZSTD_compressBound(in.size), &out)))
    {
        enif_release_binary(&in);
        return _zstd_wrap_nomem(self);
    }

    /* perform compression */
    if (ZSTD_isError(ret = ZSTD_compress(out.data, out.size, in.data, in.size, level)))
    {
        enif_release_binary(&in);
        enif_release_binary(&out);
        return _zstd_wrap_error(self, ret);
    }

    /* transfer to binary object */
    ERL_NIF_TERM binary = enif_make_binary(self, &out);
    ERL_NIF_TERM result = binary;

    /* remove unused spaces */
    if (ret < out.size)
        result = enif_make_sub_binary(self, binary, 0, ret);

    /* construct the result tuple */
    enif_release_binary(&in);
    return enif_make_tuple2(self, _atom_ok, result);
}

static ERL_NIF_TERM _zstd_simple_decompress(ErlNifEnv *self, int argc, const ERL_NIF_TERM argv[])
{
    size_t ret;
    ErlNifBinary in;
    ErlNifBinary out;

    /* extract the binary data */
    if (!(enif_inspect_iolist_as_binary(self, argv[0], &in)))
        return _zstd_wrap_inval(self);

    /* find the output size */
    if (!(ret = ZSTD_getDecompressedSize(in.data, in.size)))
    {
        enif_release_binary(&in);
        return _zstd_wrap_string(self, "Unable to determain decompressed size");
    }

    /* allocate output binary */
    if (!(enif_alloc_binary(ret, &out)))
    {
        enif_release_binary(&in);
        return _zstd_wrap_nomem(self);
    }

    /* perform decompression */
    if (ZSTD_isError(ret = ZSTD_decompress(out.data, out.size, in.data, in.size)))
    {
        enif_release_binary(&in);
        enif_release_binary(&out);
        return _zstd_wrap_error(self, ret);
    }

    /* construct the result tuple */
    enif_release_binary(&in);
    return enif_make_tuple2(self, _atom_ok, enif_make_binary(self, &out));
}

ERL_NIF_INIT(
    Elixir.Zstd,        /* module name */
    _zstd_exports,      /* export table */
    _zstd_on_load,      /* on load */
    _zstd_on_reload,    /* on reload */
    _zstd_on_upgrade,   /* on upgrade */
    NULL                /* on unload */
)
