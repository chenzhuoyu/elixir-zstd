defmodule ZstdTest do
  use ExUnit.Case
  doctest Zstd

  test "Simple Compression" do
    {:ok, data} = Zstd.simple_compress("hello, world")
    {:ok, orig} = Zstd.simple_decompress(data)
    assert orig == "hello, world"
  end

  test "Stream Decompression" do
    zcs = Zstd.cstream_new()
    Zstd.cstream_init(zcs)
    {:ok, data} = Zstd.stream_compress(zcs, "hello, world")
    {:ok, tail} = Zstd.stream_flush(zcs)

    zds = Zstd.dstream_new()
    Zstd.dstream_init(zds)
    {:ok, orig} = Zstd.stream_decompress(zds, [data, tail])
    assert orig == "hello, world"
  end
end
