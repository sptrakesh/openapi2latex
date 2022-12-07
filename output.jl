function table(i::Info)::String
    if isempty(i.termsOfService.scheme) && isempty(i.version) && isempty(i.license.name) return "" end
    s = """\\section{Other Information}
\\begin{table}[ht]
\\centering
\\begin{tabular}{|l|l|}
"""
    if !isempty(i.termsOfService.scheme) s = s * "\\hline Terms Of Service & \\href{$(uristring(i.termsOfService))}{$(uristring(i.termsOfService))} \\\\\n" end
    if !isempty(i.version) s = s * "\\hline Version & $(i.version) \\\\\n" end
    if !isempty(i.license.name) && !isempty(i.license.identifier) s = s * "\\hline License & $(i.license.identifier) \\\\\n"
    elseif !isempty(i.license.name) && !isempty(i.license.url.scheme) s = s * "\\hline License & \\href{$(i.license.name)}{$(uristring(i.license.url))} \\\\\n"
    end
    s * """\\hline
\\end{tabular}
\\caption{API Information}
\\end{table}
"""
end

function servers(o::OpenAPI)::String
    if isempty(o.servers) return "" end

    t = """\\begin{table}[ht]
\\centering
\\begin{tabular}{|l|l|}
"""

    for s in o.servers
        t = t * "\\hline $(s.description) & \\href{$(uristring(s.url))}{$(uristring(s.url))} \\\\\n"
    end

    t * """\\hline
\\end{tabular}
\\caption{Server Information}
\\end{table}
"""
end


function security_schemes(c::Components)::String
    if isempty(c.securitySchemes) return "" end

    t = """\\begin{table}[ht]
\\centering
\\begin{tabular}{|l|l|l|}
"""
    for (key, s) in c.securitySchemes
        count = 0
        if !isempty(s.type) count += 1 end
        if !isempty(s.description) count += 1 end
        if !isempty(s.name) count += 1 end
        if !isempty(s.in) count += 1 end
        if !isempty(s.scheme) count += 1 end
        if !isempty(s.bearerFormat) count += 1 end
        if !isempty(s.openIdConnectUrl.scheme) count += 1 end
        t = t * "\\hline \\multirow{$count}{*}{\\textbf{$key}} & Type & $(s.type) \\\\\n"
        if !isempty(s.description) t = t * "\\cline{2-3} & Description & $(s.description) \\\\\n" end
        if !isempty(s.name) t = t * "\\cline{2-3} & Name & $(s.name) \\\\\n" end
        if !isempty(s.in) t = t * "\\cline{2-3} & In & $(s.in) \\\\\n" end
        if !isempty(s.scheme) t = t * "\\cline{2-3} & Scheme & $(s.scheme) \\\\\n" end
        if !isempty(s.bearerFormat) t = t * "\\cline{2-3} & Bearer Format & $(s.bearerFormat) \\\\\n" end
        if !isempty(s.openIdConnectUrl.scheme) t = t * "\\cline{2-3} & OpenId Connect & \\href{$(uristring(s.openIdConnectUrl))}{$(uristring(s.openIdConnectUrl))} \\\\\n" end
    end

    t * """\\hline
\\end{tabular}
\\caption{Security Schemes}
\\end{table}
"""
end

function generate(o::OpenAPI, fp::String, author::String, input::String)
    collect_tags!(o)
    collect_paths!(o, input)

    head = """\\input{$(pwd())/structure.tex}

\\title{$(o.info.title)}
\\author{$(author)}
\\date{\\today}

\\begin{document}

\\frontmatter
% Title Page
\\thispagestyle{empty}
\\maketitle

%\\newpage
\\tableofcontents

\\listoftables

\\clearpage
\\mainmatter
\\chapter{Information}
\\epigraph{$(o.info.summary)}

$(o.info.description)

$(table(o.info))

$(servers(o))

$(security_schemes(o.components))

\\part{Endpoints}
"""

    open(fp, "w") do f
        write(f, head)

        for t in o.tags
            write(f, "\n\\chapter{$(t.name)}\n")
            if !isempty(t.description) write(f, "\\epigraph{$(t.description)}\n") end
        end

        write(f, """\\backmatter
\\bibliography{bibliography} % Use the bibliography.bib file for the bibliography
\\bibliographystyle{plainnat} % Use the plainnat style of referencing
\\printindex % Print the index at the very end of the document
\\end{document}""")
    end
end
