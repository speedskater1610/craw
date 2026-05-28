// ### NOTICE
// This comes from this zig CLI template
// https://github.com/zig-utils/zig-starter/blob/main/build.zig.full

const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // Add external modules from pantry dependencies
    const cli_mod = b.addModule("zig-cli", .{
        .root_source_file = b.path("../zig-cli/src/root.zig"),
        .target = target,
    });

    const config_mod = b.addModule("zig-config", .{
        .root_source_file = b.path("../zig-config/src/root.zig"),
        .target = target,
    });

    const error_handling_mod = b.addModule("zig-error-handling", .{
        .root_source_file = b.path("../zig-error-handling/src/root.zig"),
        .target = target,
    });

    const test_framework_mod = b.addModule("zig-test-framework", .{
        .root_source_file = b.path("../zig-test-framework/src/root.zig"),
        .target = target,
    });

    // Create library module
    const lib_mod = b.addModule("starter", .{
        .root_source_file = b.path("src/lib.zig"),
        .target = target,
    });
    lib_mod.addImport("zig-config", config_mod);
    lib_mod.addImport("zig-error-handling", error_handling_mod);

    // Build executable
    const exe = b.addExecutable(.{
        .name = "zig-starter",
        .root_module = b.createModule(.{
            .root_source_file = b.path("src/main.zig"),
            .target = target,
            .optimize = optimize,
            .imports = &.{
                .{ .name = "starter", .module = lib_mod },
                .{ .name = "zig-cli", .module = cli_mod },
                .{ .name = "zig-config", .module = config_mod },
                .{ .name = "zig-error-handling", .module = error_handling_mod },
            },
        }),
    });
    b.installArtifact(exe);

    // Run step
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    // Tests
    const lib_test_mod = b.createModule(.{
        .root_source_file = b.path("src/lib.zig"),
        .target = target,
        .optimize = optimize,
        .imports = &.{
            .{ .name = "zig-config", .module = config_mod },
            .{ .name = "zig-error-handling", .module = error_handling_mod },
            .{ .name = "zig-test-framework", .module = test_framework_mod },
        },
    });

    const lib_unit_tests = b.addTest(.{
        .root_module = lib_test_mod,
    });

    const exe_test_mod = b.createModule(.{
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .imports = &.{
            .{ .name = "starter", .module = lib_mod },
            .{ .name = "zig-cli", .module = cli_mod },
            .{ .name = "zig-config", .module = config_mod },
            .{ .name = "zig-error-handling", .module = error_handling_mod },
            .{ .name = "zig-test-framework", .module = test_framework_mod },
        },
    });

    const exe_unit_tests = b.addTest(.{
        .root_module = exe_test_mod,
    });

    const run_lib_unit_tests = b.addRunArtifact(lib_unit_tests);
    const run_exe_unit_tests = b.addRunArtifact(exe_unit_tests);

    const test_step = b.step("test", "Run unit tests");
    test_step.dependOn(&run_lib_unit_tests.step);
    test_step.dependOn(&run_exe_unit_tests.step);
}
