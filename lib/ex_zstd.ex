defmodule ExZstd do
  @moduledoc """
  Elixir binding of the Zstandard library
  """

  @on_load {:__init__, 0}
  app = Mix.Project.config()[:app]

  @spec __init__() :: :ok
  def __init__ do
    dir = :code.priv_dir(unquote(app))
    :ok = :erlang.load_nif(:filename.join(dir, 'nif_ex_zstd'), 0)
  end

  @doc """
  Create a new Compression Stream
  """
  @spec cstream_new() :: reference()
  def cstream_new()
  def cstream_new(), do: exit(:nif_library_not_loaded)

  @doc """
  Create a new Decompression Stream
  """
  @spec dstream_new() :: reference()
  def dstream_new()
  def dstream_new(), do: exit(:nif_library_not_loaded)

  @doc """
  Initialize or re-initialize a Compression Stream
  using default compression level
  """
  @spec cstream_init(reference()) :: :ok | {:error, :einval | String.t()}
  def cstream_init(zcs)
  def cstream_init(_zcs), do: exit(:nif_library_not_loaded)

  @doc """
  Initialize or re-initialize a Compression Stream,
  using the compression level specified by `level`
  """
  @spec cstream_init(reference(), integer()) :: :ok | {:error, :einval | String.t()}
  def cstream_init(zcs, level)
  def cstream_init(_zcs, _level), do: exit(:nif_library_not_loaded)

  @doc """
  Initialize or re-initialize a Decompression Stream
  """
  @spec dstream_init(reference()) :: :ok | {:error, :einval | String.t()}
  def dstream_init(zds)
  def dstream_init(_zds), do: exit(:nif_library_not_loaded)

  @doc """
  Reset a Compression Stream and start a new compression job,
  using same parameters from previous job
  """
  @spec cstream_reset(reference()) :: :ok | {:error, :einval | String.t()}
  def cstream_reset(zcs)
  def cstream_reset(_zcs), do: exit(:nif_library_not_loaded)

  @doc """
  Reset a Compression Stream and start a new compression job with pledged source size,
  using same parameters from previous job
  """
  @spec cstream_reset(reference(), non_neg_integer()) :: :ok | {:error, :einval | String.t()}
  def cstream_reset(zcs, size)
  def cstream_reset(_zcs, _size), do: exit(:nif_library_not_loaded)

  @doc """
  Reset a Decompression Stream and start a new compression job,
  using same parameters from previous job
  """
  @spec dstream_reset(reference()) :: :ok | {:error, :einval | String.t()}
  def dstream_reset(zds)
  def dstream_reset(_zds), do: exit(:nif_library_not_loaded)

  @doc """
  Flush the Compression Stream, then close it
  """
  @spec stream_flush(reference()) :: {:ok, binary()} | {:error, :einval | :enomem | String.t()}
  def stream_flush(zcs)
  def stream_flush(_zcs), do: exit(:nif_library_not_loaded)

  @doc """
  Compress a chunk of data with Compression Stream
  """
  @spec stream_compress(reference(), binary()) ::
          {:ok, binary()} | {:error, :einval | :enomem | String.t()}
  def stream_compress(zcs, data)
  def stream_compress(_zcs, _data), do: exit(:nif_library_not_loaded)

  @doc """
  Decompress a chunk of data with Decompression Stream
  """
  @spec stream_decompress(reference(), binary()) ::
          {:ok, binary()} | {:error, :einval | :enomem | String.t()}
  def stream_decompress(zcs, data)
  def stream_decompress(_zcs, _data), do: exit(:nif_library_not_loaded)

  @doc """
  Compress a chunk of data with Simple API
  """
  @spec simple_compress(binary()) :: {:ok, binary()} | {:error, :einval | :enomem | String.t()}
  def simple_compress(data)
  def simple_compress(_data), do: exit(:nif_library_not_loaded)

  @doc """
  Compress a chunk of data with Simple API, using specified compression level
  """
  @spec simple_compress(binary(), integer()) ::
          {:ok, binary()} | {:error, :einval | :enomem | String.t()}
  def simple_compress(data, level)
  def simple_compress(_data, _level), do: exit(:nif_library_not_loaded)

  @doc """
  Decompress a chunk of data with Simple API
  """
  @spec simple_decompress(binary()) :: {:ok, binary()} | {:error, :einval | :enomem | String.t()}
  def simple_decompress(data)
  def simple_decompress(_data), do: exit(:nif_library_not_loaded)
end
