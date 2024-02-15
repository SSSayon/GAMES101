
#let conf(doc) = {
  set page(
    paper: "a4",
    numbering: "1",
  )

  set heading(numbering: "1.1. ")  

  // font
  set text(font: ("linux libertine", "STSong"), lang: "zh", region: "cn")
  show emph: text.with(font: ("linux libertine", "STKaiti"))
  show strong: text.with(font: ("linux libertine", "STHeiti"))

  // code
  show raw.where(block: false): box.with(
    fill: luma(240),
    inset: (x: 3pt, y: 0pt),
    outset: (y: 3pt),
    radius: 2pt,
  )

  show raw.where(block: true): it => block( 
    // Layout
    width: 100%,
    fill: rgb("f2f2f2"),
    outset: (y: 5pt),
    radius: 1pt,
    stroke: (1.5pt + rgb("e6e6e6")),
  )[
    // Title's layout
    #block(
      width: 100%,
      fill: rgb("e6e6e6"),
      outset: (y: 5pt),
      radius: 0pt,
    )[#" " #str(it.lang)]

    // Line numbering
    #let num = it.lines.len()
    #let arr = ()
    #for i in range(1, num+1).map(str) {
      if i.len() <= str(num).len() {
        i = (str(num).len()-i.len()+1) * " " + i
      }
      arr.push(i)
    }

    // Fill in elements
    #grid(
      columns: (1.5em + str(num).len() * 1em, auto),
      [
        #set text(
          fill: rgb("8e8a8a")
        )
        #grid(
          rows: num * (12.4pt,),
          ..arr
        )
      ],
      it,
    )
  ]

  // link & reference
  show link: link => {
    set text(fill: rgb("#ee0000"))
    underline(link)
  }

  set ref(supplement: it => {""})
  show ref: it => {
      let el = it.element
      set text(fill: rgb("#ee0000"))
      if (el == none) [??] else { it }
  }

  // args
  doc
}

// font
#let bold_italic(txt) = {
  set text(font: ("linux libertine", "STFangsong"))
  txt
}

// table
#import "@preview/tablex:0.0.6": tablex, hlinex
#import "@preview/tablem:0.1.0": tablem
#let three-line-table = tablem.with(
  render: (..args) => {
    tablex(
      columns: auto,
      auto-lines: false,
      align: center + horizon,
      hlinex(y: 0),
      hlinex(y: 1, stroke: 0.5pt),
      ..args,
      hlinex(),
    )
  }
)
#let table3(args, cap) = figure(
  three-line-table(args),
  supplement: [表],
  kind: table,
  caption: {
    if cap == [] { none } else { cap }
  }
)

// math
#let math_block(case, args) = rect(
  width: 100%,
  fill: if case == "注记" {luma(240)} 
   else if case == "定义" {rgb("e3ffe3")}
   else if case == "命题" {rgb("ccebff")}
      else                {rgb("c3ffcf")},
  stroke: if case == "注记" {(left: 1.5pt + black)} 
     else if case == "定义" {(left: 1.5pt + rgb("00a652"))}
     else if case == "命题" {(left: 1.5pt + rgb("00aef7"))}
         else               {(left: 1.5pt + rgb("007f00"))},
)[
  *#case.* #math.space
  #if case != "注记" {
    emph(args)
  } else {
    args
  }
]

#let proof(args) = [
  #box(width: 2.5pt) _证明._ #math.space
  #args
  #align($square$, right)
]
