const std = @import("std");

pub fn build(b: *std.Build) void {
    const mod = b.createModule(.{
        .root_source_file = b.path("src/root.zig"),
        .target = b.standardTargetOptions(.{}),
    });

    const lib = b.addLibrary(.{
        .linkage = .static,
        .name = "fence",
        .root_module = mod,
    });
    // https://github.com/ziglang/zig/issues/6817
    lib.bundle_compiler_rt = true;

    b.installArtifact(lib);

    const header_file = b.addInstallHeaderFile(b.path("src/fence.h"), "fence.h");
    b.getInstallStep().dependOn(&header_file.step);
}
