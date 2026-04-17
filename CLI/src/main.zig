const tagSys = @import("tag.zig");
const std = @import("std");

pub fn main(init: std.process.Init) !void {
    var gpa = std.heap.DebugAllocator(.{}){};
    const allocator = gpa.allocator();
    
    var arena = std.heap.ArenaAllocator.init(gpa.allocator());
    defer arena.deinit();

    const args = try init.args.toSlice(arena.allocator());

    var tags = args[0..args.len]; // get rid of the program name (crawcli)

    for (tags) |tag| {
        tagSys.getTag(tag);
    }
}