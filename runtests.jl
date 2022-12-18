using Test

include("model.jl")

@testset "Conversion tests" begin
    @testset "Style conversion tests" begin
        @testset "Bold with asterisks" begin
            line = "Micro-services for **ReelSense** data interactions."
            delim = "**"
            out = model.convert_style(line, delim, "\\textbf")
            @test "Micro-services for \\textbf{ReelSense} data interactions." == out
        end

        @testset "Bold with underscore" begin
            line = "Micro-services for __ReelSense__ data interactions."
            delim = "__"
            out = model.convert_style(line, delim, "\\textbf")
            @test "Micro-services for \\textbf{ReelSense} data interactions." == out
        end

        @testset "Italicised text with asterisks" begin
            line = "*Any* logged in user may invoke this endpoint."
            delim = "*"
            out = model.convert_style(line, delim, "\\textit")
            @test "\\textit{Any} logged in user may invoke this endpoint." == out
        end

        @testset "Italicised text with underscore" begin
            line = "_Any_ logged in user may invoke this endpoint."
            delim = "_"
            out = model.convert_style(line, delim, "\\textit")
            @test "\\textit{Any} logged in user may invoke this endpoint." == out
        end

        @testset "Bullet list with *" begin
            line = """* Item 1
* Item 2
* Item 3
"""
            out = model.convert_bullets(line)
            @test contains(out, "\\begin{itemize}")
            @test contains(out, "\\item Item 1")
            @test contains(out, "\\item Item 2")
            @test contains(out, "\\item Item 3")
            @test contains(out, "\\end{itemize}")
        end

        @testset "Bullet list with -" begin
            line = """- Item 1
- Item 2
- Item 3
"""
            out = model.convert_bullets(line)
            @test contains(out, "\\begin{itemize}")
            @test contains(out, "\\item Item 1")
            @test contains(out, "\\item Item 2")
            @test contains(out, "\\item Item 3")
            @test contains(out, "\\end{itemize}")
        end

        @testset "Bullet list with 2 levels" begin
            line = """* Item 1
* Item 2
  * Item 2 a
  * Item 2 b
* Item 3
"""
            out = model.convert_bullets(line)
            @test contains(out, "\\begin{itemize}")
            @test contains(out, "\\item Item 1")
            @test contains(out, "\\item Item 2")
            @test contains(out, "\\item Item 2 a")
            @test contains(out, "\\item Item 2 b")
            @test contains(out, "\\item Item 3")
            @test contains(out, "\\end{itemize}")

            r1 = findfirst("\\begin", out)
            r2 = findlast("\\begin", out)
            @test r1 != r2

            r1 = findfirst("\\end", out)
            r2 = findlast("\\end", out)
            @test r1 != r2
        end

        @testset "Bullet list with 3 levels" begin
            line = """* Item 1
* Item 2
  * Item 2 a
  * Item 2 b
    * Item 2 b i
    * Item 2 b ii
"""
            out = model.convert_bullets(line)
            @test contains(out, "\\begin{itemize}")
            @test contains(out, "\\item Item 1")
            @test contains(out, "\\item Item 2")
            @test contains(out, "\\item Item 2 a")
            @test contains(out, "\\item Item 2 b")
            @test contains(out, "\\item Item 2 b i")
            @test contains(out, "\\item Item 2 b ii")
            @test contains(out, "\\end{itemize}")

            r1 = findfirst("\\begin", out)
            r2 = findlast("\\begin", out)
            @test r1 != r2

            r1 = findfirst("\\end", out)
            r2 = findlast("\\end", out)
            @test r1 != r2

            r = findall("\\begin{itemize}", out)
            @test size(r)[1] == 3

            r = findall("\\end{itemize}", out)
            @test size(r)[1] == 3
        end
    end
end