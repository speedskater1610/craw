const tagSys = @import("tag.zig");
const std = @import("std");

pub fn main() !void {
    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    
    var arena = std.heap.ArenaAllocator.init(gpa.allocator());
    defer arena.deinit();

    const args = try init.args.toSlice(arena.allocator());

    var tags = args[0..args.len]; // get rid of the program name (crawcli)

    for (tags) |tag| {
        tagSys.getTag(tag);
    }
}