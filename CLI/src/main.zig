const std = @import("std");
const cli = @import("zig_cli");
const config = @import("zig_config");
const errors = @import("zig_error_handling");
const starter = @import("starter");

const AppConfig = struct {
    verbose: bool = false,
    output: []const u8 = "output.txt",
    max_retries: u32 = 3,
};

pub fn main() !void {
    var gpa: std.heap.DebugAllocator(.{}) = .init;
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    // Initialize CLI
    var app = cli.App.init(allocator, "zig-starter", "A Zig starter template");
    defer app.deinit();

    // Add version flag
    try app.addFlag("version", .{
        .short = 'v',
        .description = "Print version information",
        .action = .{ .store_true = {} },
    });

    // Add verbose flag
    try app.addFlag("verbose", .{
        .description = "Enable verbose output",
        .action = .{ .store_true = {} },
    });

    // Add input argument
    try app.addArgument("input", .{
        .description = "Input file to process",
        .required = false,
    });

    // Add process command
    var process_cmd = try app.addCommand("process", .{
        .description = "Process input data",
        .handler = processCommand,
    });

    try process_cmd.addArgument("file", .{
        .description = "File to process",
        .required = true,
    });

    try process_cmd.addFlag("output", .{
        .short = 'o',
        .description = "Output file path",
        .action = .{ .store = "output.txt" },
    });

    // Add config command
    var config_cmd = try app.addCommand("config", .{
        .description = "Show configuration",
        .handler = configCommand,
    });

    try config_cmd.addFlag("show", .{
        .description = "Show current configuration",
        .action = .{ .store_true = {} },
    });

    // Parse and run
    try app.parse();
    try app.run();
}

fn processCommand(ctx: *cli.Context) !void {
    const allocator = ctx.allocator;
    const file_path = ctx.getArgument("file") orelse return error.MissingArgument;
    const output_path = ctx.getFlag("output") orelse "output.txt";

    const stdout = std.io.getStdOut().writer();
    try stdout.print("Processing file: {s}\n", .{file_path});
    try stdout.print("Output will be written to: {s}\n", .{output_path});

    // Read input file
    const file = std.fs.cwd().openFile(file_path, .{}) catch |err| {
        try stdout.print("Error opening file: {}\n", .{err});
        return err;
    };
    defer file.close();

    const content = file.readToEndAlloc(allocator, 1024 * 1024) catch |err| {
        try stdout.print("Error reading file: {}\n", .{err});
        return err;
    };
    defer allocator.free(content);

    // Process using library
    var lib = try starter.StarterLib.init(allocator);
    defer lib.deinit();

    const result = lib.processData(content);
    if (result.isErr()) {
        try stdout.print("Processing failed: {}\n", .{result.unwrapErr()});
        return error.ProcessingFailed;
    }

    const processed = result.unwrap();
    defer allocator.free(processed);

    // Write output
    const output_file = try std.fs.cwd().createFile(output_path, .{});
    defer output_file.close();

    try output_file.writeAll(processed);
    try stdout.print("Successfully processed {d} bytes\n", .{processed.len});
}

fn configCommand(ctx: *cli.Context) !void {
    const stdout = std.io.getStdOut().writer();

    try stdout.writeAll("Current Configuration:\n");
    try stdout.writeAll("=====================\n");
    try stdout.writeAll("Name: zig-starter\n");
    try stdout.writeAll("Version: 0.1.0\n");
    try stdout.writeAll("Zig Version: 0.15.1\n");
    try stdout.writeAll("\nDependencies:\n");
    try stdout.writeAll("  - zig-cli\n");
    try stdout.writeAll("  - zig-config\n");
    try stdout.writeAll("  - zig-error-handling\n");
    try stdout.writeAll("  - zig-test-framework\n");
    try stdout.writeAll("  - zig-bump\n");

    _ = ctx;
}

test "main configuration" {
    const testing = @import("zig_test_framework");
    const app_config = AppConfig{};

    try testing.expect(app_config.max_retries == 3);
    try testing.expectEqualStrings("output.txt", app_config.output);
}
