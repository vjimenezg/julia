using Artifacts: with_artifacts_directory
# using Pkg.Artifacts: ensure_all_artifacts_installed
using Pkg.Artifacts: load_artifacts_toml, ensure_artifact_installed
let
    tempdir = joinpath(@__DIR__, "artifacts")
    rm(tempdir; recursive=true, force=true)
    toml = joinpath(@__DIR__, "Artifacts.toml")
    unused = Base.BinaryPlatforms.Platform(string(Sys.ARCH), "linux")
    with_artifacts_directory(tempdir) do
        # ensure_all_artifacts_installed(toml; include_lazy=true)
        dict = load_artifacts_toml(toml)
        for (name, meta) in dict
            if meta isa Array
                for meta in meta
                    ensure_artifact_installed(name, meta, toml; platform=unused)
                end
            else; meta::Dict
                ensure_artifact_installed(name, meta, toml; platform=unused)
            end
        end
    end
end
