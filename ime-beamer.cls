\ProvidesClass{ime-beamer}[2013/10/09 v1.0 Modelo do Instituto Militar de Engenharia baseado em beamer]
\usepackage[utf8]{inputenc}% Codificacao do documento (conversão automática dos acentos)

%
% Repassa todos os parametros a classe abntex2,
\DeclareOption*{%
  \PassOptionsToClass{\CurrentOption}{beamer}%
}
\ProcessOptions\relax
\LoadClass{beamer}

\RequirePackage{beamerfoils}
\usetheme{Copenhagen}

\pgfdeclareimage[height=1.4cm]{logo}{IME}
\logo{\pgfuseimage{logo}}

\newcommand*\oldmacro{}%
\let\oldmacro\insertshorttitle%
\renewcommand*\insertshorttitle{%
  \oldmacro\hfill%
  \insertframenumber\,/\,\inserttotalframenumber}

\institute{Instituto Militar de Engenharia}
