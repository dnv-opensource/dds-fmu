% Class for SINTEF reports with LaTeX.
%
% by Federico Zenith, federico.zenith@sintef.no.

\ProvidesClass{sintefreport}[2021/03/15 A class to prepare reports in SINTEF]

\newcommand*{\@summaryclassification}{\EnglishNorsk{Restricted}{Fortrolig}}

\DeclareOption{unrestrictedsummary}{%%
  \renewcommand*{\@summaryclassification}{\EnglishNorsk{Unrestricted}{Åpen}}%
}
\DeclareOption{restrictedsummary}{%
  \renewcommand*{\@summaryclassification}{%
    \EnglishNorsk{Restricted}{Fortrolig}
  }
}
\DeclareOption{confidentialsummary}{%
  \renewcommand*{\@summaryclassification}{%
    \EnglishNorsk{Confidential}{Strengt fortrolig}%
  }
}

% Should not be used in reports
\DeclareOption{internal}{\OptionNotUsed}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{sintefdoc}}
\ProcessOptions\relax

\LoadClass[report]{sintefdoc}

% Set margins
\RequirePackage[top=3cm,          bottom=3cm,
                left=2cm,         right=2cm,
                headheight=2.51cm, headsep=0.5cm,
                footskip=3ex]{geometry}

% Setting up headers and footers
\RequirePackage{fancyhdr,lastpage}
\pagestyle{fancy}

% Standard header & footer
\renewcommand{\headrulewidth}{0pt}
\fancyhf{}% clear all header and footer fields
\lhead{\renewcommand{\baselinestretch}{0}%
       \raisebox{0.2mm}{\includegraphics[height=2.34cm]{\@mainlogo}}%
       }
\cfoot{%
  \color{sintefblue}%
  \@separator\newline%
  \hspace*{\tabcolsep}%
  \begin{minipage}[c]{0.25\textwidth}
    \@LabelText \@projectlabel\newline
    \@project
  \end{minipage}%
  \begin{minipage}[c]{0.25\textwidth}
    \@LabelText \@reportnumberlabel\newline
    \@reportnumber
  \end{minipage}%
  \begin{minipage}[c]{0.25\textwidth}
    \@LabelText \@versionlabel\newline
    \@version
  \end{minipage}%
  \hfill
  \thepage\ \EnglishNorsk{of}{av} \PlainPageRef{LastPage}%
  \hspace*{\tabcolsep}\newline%
  \@separator
}

% plain style is conjured by first pages of new chapters
% empty definition makes it a clone of fancy
\fancypagestyle{plain}{}

\fancypagestyle{coverpage}{%
  \fancyhf{}% clear all header and footer fields
  \lhead{\renewcommand{\baselinestretch}{0}%
         \hspace*{2.3cm}\raisebox{4.85mm}%
         {\includegraphics[height=3.62cm]{\@mainlogo}}%
        }
}

\fancypagestyle{firstpage}{%
  \cfoot{%
    \color{sintefblue}
    \@separator\newline
    \begin{minipage}[c]{0.4\textwidth}
    ~\\\phantom{opera}
    \end{minipage}%
    \hfill
    \begin{minipage}[t]{0.4\textwidth}
      \flushright%
      \thepage\ \EnglishNorsk{of}{av} \PlainPageRef{LastPage}
    \end{minipage}%
    \hspace*{\tabcolsep}\newline%
    \@separator
  }
}

\fancypagestyle{backcover}{%
  \fancyhf{}% clear all header and footer fields
  \lhead{\renewcommand{\baselinestretch}{0}%
         \raisebox{0.2mm}{\includegraphics[height=2.34cm]{%
         SINTEF_Logo_Sentrert_Negativ_\ifbool{print}{CMYK}{RGB}%
         }}%
        }
}

\newcommand{\@coverbackground}{}
\newcommand{\coverbackground}[1]{\renewcommand{\@coverbackground}{#1}}

\newcommand{\@clientlogo}{}
\newcommand{\clientlogo}[1]{\renewcommand{\@clientlogo}{#1}}

% Title page, first page, table of contents etc.
\RequirePackage{eso-pic,sintefhistory,sintefattachments,tikz}
\newcommand{\frontmatter}{%
  \newgeometry{top=4.75cm,        bottom=0cm,
               left=0cm,          right=0cm,
               headheight=4.25cm, headsep=5mm}
  \thispagestyle{coverpage}%
  \pagecolor{sintefgrey}
  \ifcsempty{@coverbackground}{}{%
    \AddToShipoutPictureBG*{\includegraphics[height=\paperheight,%
    keepaspectratio]{\@coverbackground}}
  }
  \noindent%
  \hspace*{2.3cm}%
  \begin{minipage}[t]{13cm}
    \vspace*{7.5cm}%
    \color{sintefblue}%
    \pgfsetfillopacity{0.75}%
    \colorbox{white}{
    \pgfsetfillopacity{1}%
    \begin{minipage}[t][4.16cm]{12.24cm}
    \fontsize{40}{40}\selectfont\vspace*{1ex}\bfseries\@reportlabel%
    \end{minipage}%
    }%

    \vspace{1ex}
    \pgfsetfillopacity{0.75}%
    \colorbox{white}{
      \pgfsetfillopacity{1}%
      \begin{minipage}[t][6.83cm]{12.24cm}
        \raggedright\bfseries
        \Huge\vspace*{1ex}\@title
    
        \normalsize\@subtitle
    
        \vspace{3ex}\LARGE\@authorlabel\\
        {\Large\mdseries\@author}
    
       \vspace{2ex}\@reportnumberlabel\\
       \Large\mdseries\@reportnumber~---~\@classification
      \end{minipage}%
    }

    \vspace{1ex}
    \pgfsetfillopacity{0.75}%
    \colorbox{white}{
      \pgfsetfillopacity{1}%
      \begin{minipage}[t][2.57cm]{12.24cm}
        \raggedright
        \LARGE\vspace*{1ex}\textbf{\@clientlabel}\\
        \Large\@client\\
        \@clientlogo
      \end{minipage}%
    }%
  \end{minipage}%
  %
  % Use "manual" footer instead of fancyhdr's \cfoot
  % fancyhdr does not talk to geometry and will not be aware of the special
  % layout of the first page.
  \vfill\noindent%
  \colorbox{sintefblue}{%
  \begin{minipage}[t][1.68cm]{.99\textwidth}%
  \LARGE\vspace*{2ex}\raggedleft%
  \color{white}\textbf{\@motto}\hspace*{1.5em}%
  \end{minipage}%
  }%
  \addtocounter{page}{-1}%
  \ifbool{print}{% Add empty back page to cover for print version
    \newpage
    \thispagestyle{empty}
    \phantom{opera}
    \addtocounter{page}{-1}
  }{}
  \restoregeometry%
  \nopagecolor%
% -------- END OF COVER PAGE --------

% -------- START FIRST PAGE --------
  \thispagestyle{firstpage}%
  \noindent%
  % Use 0.99\textheight as full \textheight can cause odd rounding errors.
  \begin{minipage}[t][0.99\textheight]{0.3\textwidth}
    \flushleft{\small\@info}
    \vspace{5ex}
    \@LabelText\@keywordlabel:\\
    \@keywords
    
    \vfill
    \includegraphics[width=\textwidth]{isoinscription}
  \end{minipage}%
  \begin{minipage}[t][0.99\textheight]{0.7\textwidth}
    \setlength{\parskip}{2ex}%
    \raggedright%
    \Huge\@reportlabel

    \vspace{2ex}
    \huge{\bfseries\@title}

    \normalsize \@subtitle

    \vspace{4ex}%
    \noindent%
    \hspace*{\tabcolsep}
    \begin{minipage}[t]{0.5\textwidth}
      \@LabelText \@versionlabel\\
      \@version
    \end{minipage}%
    \begin{minipage}[t]{0.47\textwidth}
      \@LabelText \@datelabel\\
      \@date
    \end{minipage}\\
    \@separator\\
    \hspace*{\tabcolsep}
    \begin{minipage}[t]{0.97\textwidth}
      \@LabelText \@Authorlabel\\
      \@author
    \end{minipage}\\
    \@separator\\
    \hspace*{\tabcolsep}
    \begin{minipage}[t]{0.5\textwidth}
      \@LabelText \@clientlabel\\
      \@client
    \end{minipage}\hspace{0.05cm}%
    \begin{minipage}[t]{0.47\textwidth}
      \@LabelText \@clientreflabel\\
      \@clientref
    \end{minipage}\\
    \@separator\\
    \hspace*{\tabcolsep}
    \begin{minipage}[t]{0.5\textwidth}
      \@LabelText \@projectlabel\\
      \@project
    \end{minipage}%
    \begin{minipage}[t]{0.47\textwidth}
      \@LabelText \@pageslabel\\
      \PlainPageRef{LastPage}
      \@attachmentpages
    \end{minipage}\\
    \@separator\\
    \hspace*{\tabcolsep}
    \begin{minipage}[t]{0.97\textwidth}
      \@LabelText \@abstractlabel\\
      \@abstract
    \end{minipage}\\
    \vfill
    \@separator\\
    \hspace*{\tabcolsep}
	\begin{minipage}[t]{0.5\textwidth}
	  \@LabelText \@preparedlabel\\
	  \@prepared
	\end{minipage}%
	\begin{minipage}[t]{0.47\textwidth}
      \@LabelText{\mdseries\@signaturelabel}\bigskip
	\end{minipage}\\
	\@separator\\
	\hspace*{\tabcolsep}
	\begin{minipage}[t]{0.5\textwidth}
	  \@LabelText \@checkedlabel\\
	  \@checked
	\end{minipage}%
	\begin{minipage}[t]{0.47\textwidth}
      \@LabelText{\mdseries\@signaturelabel}\bigskip
	\end{minipage}\\
	\@separator\\
	\hspace*{\tabcolsep}
	\begin{minipage}[t]{0.5\textwidth}
	  \@LabelText \@approvedlabel\\
	  \@approved
	\end{minipage}%
	\begin{minipage}[t]{0.47\textwidth}
      \@LabelText{\mdseries\@signaturelabel}\bigskip
	\end{minipage}\\
	\@separator\\
	\hspace*{\tabcolsep}
    \begin{minipage}[t]{0.25\textwidth}
      \raggedright
      \@LabelText \@reportnumberlabel\\
      \@reportnumber
    \end{minipage}%
    \begin{minipage}[t]{0.25\textwidth}
      \ifbool{useISBN}{%
        \@LabelText{ISBN}\\
        \@ISBN%
      }{~}% Leave empty if no ISBN required
    \end{minipage}
    \begin{minipage}[t]{0.2\textwidth}
      \@LabelText \@classificationlabel\\
      \@classification
    \end{minipage}%
    \begin{minipage}[t]{0.26\textwidth}
      \@LabelText \@summaryclassificationlabel\\
      \@summaryclassification
    \end{minipage}
  \end{minipage}


  \clearpage

  \historypage

  \tableofcontents

  \attachmentsection
  
  \clearpage

  \noindent
}

\newcommand{\backmatter}{
  \clearpage%
  % Subtract one page so LastPage seems to refer to our intended last page.
  \addtocounter{page}{-1}%
  \thispagestyle{backcover}
  \pagecolor{sintefblue}%
  \vspace*{\fill}
  \centering\Huge\bfseries\color{white}\@motto
  \vspace*{\fill}
}
