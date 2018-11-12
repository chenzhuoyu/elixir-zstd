defmodule Zstd.MixProject do
  use Mix.Project

  def project do
    [
      app: :ex_zstd,
      deps: deps(Mix.env()),
      elixir: "~> 1.7",
      version: "0.1.0",
      package: package(),
      compilers: compilers(),
      source_url: "https://github.com/chenzhuoyu/elixir-zstd",
      description: description(),
      build_embedded: Mix.env() == :prod,
      start_permanent: Mix.env() == :prod
    ]
  end

  defp deps(:publish) do
    [
      {:ex_doc, ">= 0.0.0"}
    ]
  end

  defp deps(_) do
    [
      {:libzstd, "~> 1.3.7", github: "facebook/zstd", app: false}
    ]
  end

  defp package do
    [
      name: "ex_zstd",
      files: ~w(lib c_src test .formatter.exs Makefile mix.exs README.md),
      links: %{"GitHub" => "https://github.com/chenzhuoyu/elixir-zstd"},
      licenses: ["BSD"]
    ]
  end

  defp compilers do
    [
      :zstd,
      :elixir,
      :app
    ]
  end

  defp description do
    "Elixir binding of the Zstandard library"
  end
end

defmodule Mix.Tasks.Compile.Zstd do
  def run(_) do
    if match?({:win32, _}, :os.type()) do
      IO.warn("Windows is not supported")
      exit(1)
    else
      File.mkdir_p("priv")
      {result, _code} = System.cmd("make", ["priv/nif_ex_zstd.so"], stderr_to_stdout: true)
      IO.binwrite(result)
    end
  end
end
