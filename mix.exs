defmodule Zstd.MixProject do
  use Mix.Project

  def project do
    [
      app: :zstd,
      deps: deps(),
      elixir: "~> 1.7",
      version: "0.1.0",
      compilers: compilers(),
      start_permanent: Mix.env() == :prod
    ]
  end

  defp deps do
    [
      {:zstd, github: "facebook/zstd", app: false}
    ]
  end

  defp compilers do
    [
      :zstd,
      :elixir,
      :app
    ]
  end
end

defmodule Mix.Tasks.Compile.Zstd do
  def run(_) do
    if match?({:win32, _}, :os.type()) do
      IO.warn("Windows is not supported")
      exit(1)
    else
      File.mkdir_p("priv")
      {result, _code} = System.cmd("make", ["priv/nif_zstd.so"], stderr_to_stdout: true)
      IO.binwrite(result)
    end
  end
end
