const std = @import("std");
const config = @import("zig_config");
const errors = @import("zig_error_handling");

/// Core library functionality
pub const StarterLib = struct {
    allocator: std.mem.Allocator,
    config: Config,

    pub const Config = struct {
        name: []const u8 = "default",
        debug: bool = false,
        max_retries: u32 = 3,
    };

    pub fn init(allocator: std.mem.Allocator) !StarterLib {
        return StarterLib{
            .allocator = allocator,
            .config = .{},
        };
    }

    pub fn deinit(self: *StarterLib) void {
        _ = self;
    }

    /// Example function using Result type for error handling
    pub fn processData(self: *StarterLib, data: []const u8) errors.Result([]u8, ProcessError) {
        if (data.len == 0) {
            return errors.Result([]u8, ProcessError).err(ProcessError.EmptyData);
        }

        const result = self.allocator.alloc(u8, data.len) catch {
            return errors.Result([]u8, ProcessError).err(ProcessError.AllocationFailed);
        };

        @memcpy(result, data);
        return errors.Result([]u8, ProcessError).ok(result);
    }

    /// Example function with validation
    pub fn validateInput(input: []const u8) !void {
        if (input.len == 0) {
            return ProcessError.EmptyData;
        }
        if (input.len > 1024) {
            return ProcessError.DataTooLarge;
        }
    }
};

pub const ProcessError = error{
    EmptyData,
    DataTooLarge,
    AllocationFailed,
    InvalidFormat,
};

test "StarterLib initialization" {
    const testing = @import("zig_test_framework");
    var gpa: std.heap.DebugAllocator(.{}) = .init;
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var lib = try StarterLib.init(allocator);
    defer lib.deinit();

    try testing.expect(lib.config.max_retries == 3);
    try testing.expect(!lib.config.debug);
}

test "processData with valid input" {
    const testing = @import("zig_test_framework");
    var gpa: std.heap.DebugAllocator(.{}) = .init;
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var lib = try StarterLib.init(allocator);
    defer lib.deinit();

    const input = "test data";
    const result = lib.processData(input);

    try testing.expect(result.isOk());
    const data = result.unwrap();
    defer allocator.free(data);

    try testing.expectEqualStrings(input, data);
}

test "processData with empty input" {
    const testing = @import("zig_test_framework");
    var gpa: std.heap.DebugAllocator(.{}) = .init;
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    var lib = try StarterLib.init(allocator);
    defer lib.deinit();

    const result = lib.processData("");

    try testing.expect(result.isErr());
    try testing.expect(result.unwrapErr() == ProcessError.EmptyData);
}

test "validateInput" {
    const testing = @import("zig_test_framework");

    try StarterLib.validateInput("valid");

    try testing.expectError(ProcessError.EmptyData, StarterLib.validateInput(""));

    const large_input = "x" ** 2000;
    try testing.expectError(ProcessError.DataTooLarge, StarterLib.validateInput(large_input));
}
