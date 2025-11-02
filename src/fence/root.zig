const std = @import("std");
const tlds = @import("tlds.zig");
const testing = std.testing;

export fn fence_is_top_level_domain(dom: [*:0]const u8) bool {
    return isTopLevelDomain(std.mem.span(dom));
}

fn isTopLevelDomain(dom: []const u8) bool {
    for (tlds.tab) |tld| {
        if (std.mem.eql(u8, dom, tld)) return true;
    }
    return false;
}

fn isRfc822Char(c: u8) bool {
    if (!std.ascii.isAscii(c)) return false;
    switch (c) {
        0...32, 127 => return false,
        '(', ')', ',', ';', '<', '>', '"' => return false,
    }
    return true;
}

fn quoteDepth(line: []const u8) u8 {
    const ln = std.mem.trim(u8, line, &std.ascii.whitespace);
    const first = std.mem.indexOfScalar(u8, ln, '>') orelse return 0;
    if (first != 0) return 0;

    var depth: u8 = 0;
    const last = std.mem.lastIndexOfScalar(u8, ln, '>') orelse return 1;
    for (line[first .. last + 1]) |c| {
        if (c == '>') {
            if (depth == 0xff) return depth;
            depth += 1;
            continue;
        }
        if (std.ascii.isWhitespace(c)) continue;
        break;
    }
    return depth;
}

export fn quote_depth(line: [*:0]const u8) c_int {
    const n = quoteDepth(std.mem.span(line));
    return @as(c_int, n);
}

fn copyAfterLine(dst: std.fs.File, src: std.fs.File, key: []const u8) !usize {
    var rbuf: [8192]u8 = undefined;
    var rd = std.fs.File.reader(src, &rbuf).interface;
    var wbuf: [8192]u8 = undefined;
    var wr = std.fs.File.writer(dst, &wbuf).interface;
    while (true) {
        const line = try rd.takeDelimiterInclusive('\n');
        if (std.mem.startsWith(u8, line, key)) {
            break;
        }
    }
    const n = try rd.streamRemaining(&wr);
    try wr.flush();
    return n;
}

export fn fence_copy_after_line(dst: [*:0]const u8, src: [*:0]const u8, line: [*:0]const u8) usize {
    const fdst = std.fs.cwd().createFile(std.mem.span(dst), .{}) catch return 0;
    defer fdst.close();
    const fsrc = std.fs.cwd().openFile(std.mem.span(src), .{}) catch return 0;
    defer fsrc.close();
    return copyAfterLine(fdst, fsrc, std.mem.span(line)) catch 0;
}

test "quote depth" {
    const tests = [_]struct { []const u8, u8 }{
        .{ ">>> hello", 3 },
        .{ ">    >> hello", 3 },
        .{ "     > hello >", 1 },
        .{ "hello>>", 0 },
        .{ ">> hello >>", 2 },
        .{ ">  \t> hello", 2 },
        .{ ">", 1 },
        .{ "xxx > hello >>", 0 },
    };
    for (tests) |t| {
        const got = quoteDepth(t[0]);
        errdefer std.debug.print("quote depth for \"{s}\"\n", .{t[0]});
        try testing.expectEqual(t[1], got);
    }
}

test "check domains" {
    try testing.expect(isTopLevelDomain("com"));
    try testing.expect(isTopLevelDomain("69yamum") == false);

    const str = [_:0]u8{ 'c', 'o', 'm', 0 };
    try testing.expect(fence_is_top_level_domain(str[0..]));
}
