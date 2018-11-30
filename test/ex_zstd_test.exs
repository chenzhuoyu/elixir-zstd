defmodule ExZstdTest do
  use ExUnit.Case
  doctest ExZstd

  test "Simple Compression" do
    {:ok, data} = ExZstd.simple_compress("hello, world")
    {:ok, orig} = ExZstd.simple_decompress(data)
    assert orig == "hello, world"
  end

  test "Stream Decompression" do
    zcs = ExZstd.cstream_new()
    ExZstd.cstream_init(zcs)
    {:ok, data} = ExZstd.stream_compress(zcs, "hello, world")
    {:ok, tail} = ExZstd.stream_flush(zcs)

    zds = ExZstd.dstream_new()
    ExZstd.dstream_init(zds)
    {:ok, orig} = ExZstd.stream_decompress(zds, [data, tail])
    assert orig == "hello, world"
  end
end
