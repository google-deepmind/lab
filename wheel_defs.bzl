# Copyright 2021 DeepMind Technologies Limited.
#
# Fooo
#
# ============================================================================

"""Creates Python wheel file."""

def _fix(s):
    if s.startswith("python/pip_package"):
        return s.replace("python/pip_package", "deepmind_lab")
    elif s.startswith("python"):
        return s.replace("python", "deepmind_lab")
    else:
        return "deepmind_lab/{}".format(s)

def _py_wheel_impl(ctx):
    runfiles = [s[DefaultInfo].default_runfiles for s in ctx.attr.deps]
    file_inputs = [f for r in runfiles for f in r.files.to_list()] + [f for s in ctx.attr.srcs for f in s.files.to_list()]

    basename = "-".join([
        ctx.attr.distribution,
        ctx.attr.version,
        ctx.attr.python_tag,
        ctx.attr.abi,
        ctx.attr.platform,
    ])

    outfile = ctx.actions.declare_file(basename + ".whl")
    namefile = ctx.actions.declare_file(basename + ".whlname")

    ctx.actions.run(
        inputs = file_inputs + ([ctx.file.description_file] if ctx.file.description_file else []),
        outputs = [outfile, namefile],
        arguments = [
            "--name={}".format(ctx.attr.distribution),
            "--name_file={}".format(namefile.path),
            "--description_file={}".format(ctx.file.description_file.path if ctx.file.description_file else ""),
            "--version={}".format(ctx.attr.version),
            "--python_tag={}".format(ctx.attr.python_tag),
            "--abi={}".format(ctx.attr.abi),
            "--platform={}".format(ctx.attr.platform),
            "--out={}".format(outfile.path),
            "--requires=numpy (>=1.13.3)",
            "--requires=six (>=1.10.0)",
            "--extra_requires=dm_env;dmenv_module",
        ] + ["--input_file={p};{s}".format(p = _fix(f.short_path), s = f.path) for f in file_inputs if not f.short_path.startswith("..")],
        executable = ctx.executable._wheelmaker,
        progress_message = "Building wheel",
    )

    return [DefaultInfo(
        data_runfiles = ctx.runfiles(files = [outfile]),
        files = depset([outfile, namefile]),
    )]

py_wheel = rule(
    attrs = {
        "abi": attr.string(default = "none"),
        "distribution": attr.string(mandatory = True),
        "platform": attr.string(default = "any"),
        "python_tag": attr.string(mandatory = True),
        "version": attr.string(mandatory = True),
        "description_file": attr.label(allow_single_file = True, mandatory = False),
        "deps": attr.label_list(),
        "srcs": attr.label_list(allow_files = True),
        "_wheelmaker": attr.label(
            executable = True,
            cfg = "host",
            default = "@rules_python//tools:wheelmaker",
        ),
    },
    implementation = _py_wheel_impl,
)
