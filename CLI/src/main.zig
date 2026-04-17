const tagSys = @import("tag.zig");
const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    const allocator = gpa.allocator();
    defer _ = gpa.deinit();

    var args = try std.process.argsWithAllocator(allocator);
    defer args.deinit();

    const exe_name = args.next() orelse "unknown"; // most likely `./crawcli` or `crawcli`

    const tags = args[1..]; // slice and start at index 1
    for (tags) |tag| {
        tagSys.getTag(tag);
    }
}