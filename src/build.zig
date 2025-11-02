const std = @import("std");

var alloc = std.heap.DebugAllocator(.{}){};

fn cFiles(name: []const u8) ![][]const u8 {
    var dir = try std.fs.cwd().openDir(name, .{.iterate = true});
    defer dir.close();
    const dba = alloc.allocator();
    var files = std.ArrayList([]const u8).empty;
    var iter = dir.iterate();
    while (try iter.next()) |dent| {
        if (std.mem.endsWith(u8, dent.name, ".c")) {
            try files.append(dba, try std.fs.path.join(dba, &.{name, dent.name}));
        }
    }
    return files.toOwnedSlice(dba);
}

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const fence_mod = b.createModule(.{
        .root_source_file = b.path("fence/root.zig"),
        .target = target,
    });
    const fence = b.addLibrary(.{
        .name = "fence",
        .root_module = fence_mod,
    });
    // https://github.com/ziglang/zig/issues/6817
    fence.bundle_compiler_rt = true;
    const header_file = b.addInstallHeaderFile(b.path("fence/fence.h"), "fence.h");
    b.getInstallStep().dependOn(&header_file.step);

    const extlibs = [_][]const u8{
        "cairo", "glib-2.0",  "gtk-3", "gdk-3", "gdk_pixbuf-2.0", "pango-1.0",
        "iconv",
        "gnutls", "gmp", "unistring",
    };

    const common_mod = b.createModule(.{
        .target = target,
        .link_libc = true,
    });
    common_mod.linkLibrary(fence);
    for (extlibs) |l| {
        common_mod.linkSystemLibrary(l, .{});
    }
    const common = b.addLibrary(.{
        .name = "clawscommon",
        .root_module = common_mod,
    });
    common.addCSourceFiles(.{
        .files = cFiles("common") catch unreachable,
        .flags = &.{"-g"},
    });
    common.addIncludePath(b.path("zig-out/include"));

    const gtk_mod = b.createModule(.{
        .target = target,
        .link_libc = true,
    });
    for (extlibs) |l| {
        gtk_mod.linkSystemLibrary(l, .{});
    }
    gtk_mod.linkSystemLibrary("atk-1.0", .{});
    const gtk = b.addLibrary(.{
        .name = "clawsgtk",
        .root_module = gtk_mod,
    });
    gtk.addCSourceFiles(.{
        .files = cFiles("gtk") catch unreachable,
        .flags = &.{"-g"},
    });
    gtk.addIncludePath(b.path("."));
    gtk.addIncludePath(b.path("common"));

    const etpan_mod = b.createModule(.{
        .target = target,
        .link_libc = true,
    });
    for (extlibs) |l| {
        etpan_mod.linkSystemLibrary(l, .{});
    }
    etpan_mod.linkLibrary(common);
    etpan_mod.linkSystemLibrary("etpan", .{});
    etpan_mod.linkSystemLibrary("atk", .{});
    const etpan = b.addLibrary(.{
        .name = "clawsetpan",
        .root_module = etpan_mod,
    });
    etpan.addCSourceFiles(.{
        .files = cFiles("etpan") catch unreachable,
        .flags = &.{"-g"},
    });
    etpan.addIncludePath(b.path("."));
    etpan.addIncludePath(b.path("common"));

    const exe_mod = b.createModule(.{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    exe_mod.linkLibrary(fence);
    exe_mod.linkLibrary(common);
    exe_mod.linkLibrary(gtk);
    exe_mod.linkLibrary(etpan);
    for (extlibs) |l| {
        exe_mod.linkSystemLibrary(l, .{});
    }

    // This creates another `std.Build.Step.Compile`, but this one builds an executable
    // rather than a static library.
    const exe = b.addExecutable(.{
        .name = "talons",
        .root_module = exe_mod,
    });
    exe.addCSourceFiles(.{
        .files = cFiles(".") catch unreachable,
        .flags = &.{
            "-Wall",
            "-DGTK_DISABLE_DEPRECATION_WARNINGS",
            "-DGDK_DISABLE_DEPRECATION_WARNINGS",
            "-g",
            "-std=c17",
        },
    });
    exe.addIncludePath(b.path(".."));
    exe.addIncludePath(b.path("."));
    exe.addIncludePath(b.path("etpan"));
    exe.addIncludePath(b.path("gtk"));
    exe.addIncludePath(b.path("common"));
    exe.addIncludePath(b.path("zig-out/include"));
    exe.addSystemIncludePath(b.path("../../../../../usr/local/include/atk-1.0"));

    // exe.step.dependOn(&fence.step);

    // This declares intent for the executable to be installed into the
    // standard location when the user invokes the "install" step (the default
    // step when running `zig build`).
    b.installArtifact(exe);

    // This *creates* a Run step in the build graph, to be executed when another
    // step is evaluated that depends on it. The next line below will establish
    // such a dependency.
    const run_cmd = b.addRunArtifact(exe);

    // By making the run step depend on the install step, it will be run from the
    // installation directory rather than directly from within the cache directory.
    // This is not necessary, however, if the application depends on other installed
    // files, this ensures they will be present and in the expected location.
    run_cmd.step.dependOn(b.getInstallStep());

    // This creates a build step. It will be visible in the `zig build --help` menu,
    // and can be selected like this: `zig build run`
    // This will evaluate the `run` step rather than the default, which is "install".
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    const exe_unit_tests = b.addTest(.{
        .root_module = exe_mod,
    });

    const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    // Similar to creating the run step earlier, this exposes a `test` step to
    // the `zig build --help` menu, providing a way for the user to request
    // running the unit tests.
    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_exe_unit_tests.step);
}
