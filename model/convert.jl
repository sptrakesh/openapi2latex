function convert_heading(s::String)::String
    parts = split(s, "\n"; keepempty=true)
    output = Vector{String}()

    for line in parts
        if startswith(line, "# ") push!(output, "\\section{$(SubString(line, 3))}")
        elseif startswith(line, "## ") push!(output, "\\section{$(SubString(line, 4))}")
        elseif startswith(line, "### ") push!(output, "\\section{$(SubString(line, 5))}")
        elseif startswith(line, "#### ") push!(output, "\\subsection{$(SubString(line, 6))}")
        elseif startswith(line, "##### ") push!(output, "\\subsubsection{$(SubString(line, 7))}")
        else push!(output, line)
        end
    end

    join(output, "\n")
end

function convert_listing(s::String)::String
    parts = split(s, "\n"; keepempty=true)
    output = Vector{String}()
    block = false

    for line in parts
        if startswith(line, "```")
            begin
                if block
                    push!(output, "\\end{lstlisting}")
                    block = false
                else
                    push!(output, "\\begin{lstlisting}")
                    block = true
                end
            end
        else push!(output, line)
        end
    end

    join(output, "\n")
end

function convert_bullets(s::String)::String
    parts = split(s, "\n"; keepempty=true)
    output = Vector{String}()
    block = false
    indent = 1
    numindents = 0

    for line in parts
        if match(r"^\s*\* ", line) !== nothing
            r = findfirst("*", line)
            if !block
                push!(output, "\\begin{itemize}\n")
                block = true
            elseif r[1] > indent
                push!(output, "\\begin{itemize}\n")
                numindents += 1
            elseif r[1] < indent
                push!(output, "\\end{itemize}\n")
                numindents -= 1
            end
            indent = r[1]
            push!(output, "\\item $(SubString(line, r[1]+2))")
        elseif match(r"^\s*- ", line) !== nothing
            r = findfirst("-", line)
            if !block push!(output, "\\begin{itemize}\n"); block = true end
            push!(output, "\\item $(SubString(line, r[1]+2))")
        elseif block && isempty(line)
            block = false
            push!(output, "\\end{itemize}\n")
            while numindents > 0
                push!(output, "\\end{itemize}\n")
                numindents -= 1
            end
        else
            push!(output, line)
        end
    end

    join(output, "\n")
end

function convert_link(s::String)::String
    out = s
    m = match(r"\[(.+)\]\((.+)\)", out)

    while m !== nothing
        if m.offset > 1
            rep = "$(SubString(out, 1, m.offset - 1))\\href{$(m[2])}{$(m[1])}$(SubString(out, m.offsets[2]+length(m[2])+1))"
            out = rep
        else
            rep = "\\href{$(m[2])}{$(m[1])}$(SubString(out, m.offsets[2]+length(m[2])+1))"
            out = rep
        end
        m = match(r"\[(.+)\]\((.+)\)", out)
    end

    out
end

function convert_style(line::String, delim::String, cmd::String)::String
    out = ""
    r = findfirst(delim, line)
    if r === nothing return line end

    start = r[1] > 1 ? 1 : r[1] + length(delim)
    r1 = r[1] > 1 ? r[1] - 1 : r[1]
    if r1 < start r1 = start end
    r2 = length(r) > 1 ? r[2] + 1 : r[1] + length(delim)

    e = findnext(delim, line, r2)
    if e === nothing
        @debug "No succeeding $delim after $r2."
        return line
    end

    e2 = length(e) > 1 ? e[2] + 1 : e[1] + length(delim)
    if start == r1
        out = "$cmd{$(SubString(line, r2, e[1]-1))}$(SubString(line, e2))"
    else
        out = "$(SubString(line, start, r1))$cmd{$(SubString(line, r2, e[1]-1))}$(SubString(line, e2))"
    end

    return convert_style(out, delim, cmd)
end

function convert_style(line::String)::String
    parts = Vector{String}()

    for part in split(line, "\n\n")
        out = convert_style("$part", "`", "\\texttt")
        out = convert_style(out, "**", "\\textbf")
        out = convert_style(out, "__", "\\textbf")
        out = convert_style(out, "*", "\\textit")
        out = convert_style(out, "_", "\\textit")

        push!(parts, replace(out, "_" => "\\textunderscore "))
    end

    join(parts, "\n\n")
end

function convert(s::String)::String
    processed = replace(s, "\$" => "\\\$")
    processed = convert_heading(processed)
    processed = convert_listing(processed)
    processed = convert_bullets(processed)
    processed = convert_link(processed)
    processed = replace(processed, "#" => "\\#")
    processed = replace(processed, "%" => "\\%")

    return convert_style(processed)
end
