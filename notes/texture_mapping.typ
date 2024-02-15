#import "conf/conf.typ": *
#show: conf

#align(center)[#text(1.8em)[*Texture Mapping*]]

#figure(
  image("img/texture_mapping.jpg", width: 80%)
)

= Texture Coordinate Functions

== Geometrically Determined Coordinates
Geometrically determined texture coordinates are used for simple shapes or special situations, as a quick solution, or as a starting point for designing a hand-tweaked texture coordinate map.
- spherical coordinate $phi.alt(x, y, z)=(1/(2pi)(pi+"arctan2"(y, x)), 1/pi (pi-arccos z/sqrt(x^2+y^2+z^2)))$，在两个 poles 处会有 distortion。
- cubemaps，分别定义 6 个面的 mapping，计算开销小 (例如 +z 表面 $phi.alt(x,y,z)=(x/z,y/z)$)。注意 6 个面的 uv 坐标选取的 convention。

== Interpolated Texture coordinates
最基本的三角面片 (mesh)。
- barycentric  interpolation 重心插值
#stack(
  dir: ltr,
  spacing: 1em,
  image("img/barycentric.jpg", width: 50%),
  text(0.8em)[公式推导：
    $ cases(x&=alpha x_1 &+ beta x_2 &+ (1-alpha-beta) x_3, y&=alpha y_1 &+ beta y_2 &+ (1-alpha-beta) y_3) $
    $ <==> cases((x_1-x_3)alpha&+(x_2-x_3)beta&=x-x_3, (y_1-y_3)alpha&+(y_2-y_3)beta&=y-y_3) $
    $ <==> T vec(alpha, beta)=vec(x-x_3,y-y_3), T=mat(x_1-x_3, x_2-x_3; y_1-y_3, y_2-y_3) $ 
    $ <==> vec(alpha, beta)=T^(-1)vec(x-x_3,y-y_3) $
  ] 
)

== Messes
- Tiling, Wrapping Modes。用 clamping(截断在 [0,1] 范围内) 或 wraparound indexing (取模)。
- 连续性问题。shared-vertex meshes 自然连续，但如果 map 地图这样的 texture，两端的三角形为了连接需要穿过整张地图，导致接缝 (seam) 处出问题 (整张地图挤在一起)，见下图左。
#figure(
  image("img/continuity.jpg",width: 95%)
)
解决方案：对于位于 seam 处的 vertex，一个 3D 中的点对应多个 texture 中的点 (该例为两个)，插值时，不同的三角形采用对应的不同的点。见上图右。



= Antialiasing Texture Lookups
从 texture 中采样的过程就会有失真，因此需要纹理过滤。

通常有两种情况：
- 3D空间中一个 pixel 的 footprint < texel (纹理被缩小，upsampling)
- 纹理被放大 (downsampling)

#figure(
  image("img/footprint.jpg", width: 60%)
)
如图，用 $bold(r_1)=(u_x, v_x), bold(r_2)=(u_y, v_y)$ 张成的平行四边形代替实际的 footprint。

== Nearest-neighbor Interpolation
不用多说，问题多多。

== Bilinear Interpolation
对于第一种情况，双线性插值足够了。

#rect(width: 100%, fill: luma(240), stroke: (left: 1.5pt + black))[
  #text(0.8em)[_(虎书274)_ After reading Chapter 10, you may complain that linear interpolation may not be a smooth enough reconstruction for some demanding applications. However, it can always be made good enough *by resampling the texture to a somewhat higher resolution using a better filter*, so that the texture is smooth enough that bilinear interpolation works well.]
]

#figure(
  image("img/bilinear_interpolation.jpg", width: 60%),
)
如上图，pixel 中心映到红点，则用 4 个 texel 中心插值即可。

== Mipmapping & Trilinear Interpolation
对于第二种情况，将所有位于某个 pixel 的 footprint 中的 texel 全加起来取平均，代价太大。

因此，先对 texture image 做一点预处理，建立 mipmap (即 image pyramid) 如下：

#figure(
  image("img/mipmap.jpg", width: 65%)
)
baselevel (level 0) 就是原图，然后每层 downsample$times$2，则第 $k$ 层每格宽 $2^k$ 个像素。

根据 footprint 的大小 $D$ 来选择使用哪层的 mipmap：$D=2^k ==> k=log_2 D$。$D$ 可用公式
$ D = max{sqrt(((diff u)/(diff x))^2+((diff v)/(diff x))^2), sqrt(((diff u)/(diff y))^2+((diff v)/(diff y))^2)} $
估计。公式含义见这节开头的图片理解。

由于 $k$ 不是整数，因此需用 $floor(k)$ 和 $floor(k)+1$ 两层的 mipmap 进行插值。$==>$ 三(2+1)线性插值

== Anisotropic Filtering
第二种情况，如果 footprint 很狭长，trilinear 仍会导致这些地方 overblur (因为采样了很上层的 mipmap)。如下图 3 中的远处，很模糊。
#figure(
  image("img/diff_antialias_stategies.jpg", width: 80%)
)
因此考虑用 *shortest* axis of the footprint 来选择 mipmap (min, 上面是 max)，同时则需要多次采样 (在长边采样点自然就多) 后取平均。这就是各向异性过滤 (anisotropic)。

=== A simple approach
详见 #link("https://zhuanlan.zhihu.com/p/143377682")，不见得非常正确。
#rect(width: 100%, fill: luma(240), stroke: (left: 1.5pt + black))[#text(0.8em)[
  令 $bold(r_1) = (u_x, v_x), bold(r_2) = (u_y, v_y)$。
  
  - 取 footprint 大小 $D = min{||bold(r_1)||, ||bold(r_2)||, ||bold(r_1)+bold(r_2)||, ||bold(r_1)-bold(r_2)||}$，mipmap level $k=log_2 D$。这里用向量的横纵坐标的和差的绝对值取 max 来近似模长，简化后续计算。
  - 各向异性比例
  $ N=min{2^floor(log_2 max{||bold(r_1)||, ||bold(r_2)||}/D), "maxAniso"} quad ("偶数"), $
  其中 maxAniso 作为限制，为开启的最大各向异性过滤级别 (例如游戏中会有x8, x16 这样的设置选项(?))。
  - 计算采样坐标范围：令 $bold(r)=max{bold(r_1), bold(r_2)}$，$d u=(bold(r).x)/2^k 1/N, d v=(bold(r).y)/2^k 1/N$，则采样点为
    $ u_n = u + 1/2 n dot d u, quad v_n = v + 1/2 n dot d v , $
    其中 $n = -N+1,-N+3,dots,N-3, N-1$。
  - 对每个采样点进行三线性采样，用合适的 filter 加权平均。

  #text(fill: luma(120), font: ("STKaiti"))[
    几个没懂的地方：
    1. 几处计算上的数值代入，为啥用 $2^floor(log_2 max{||bold(r_1)||, ||bold(r_2)||}/D)$ 而不用 $max{||bold(r_1)||, ||bold(r_2)||}\/D$，用 $2^k$ 而不用 $D$ 等等。感觉是要保证是偶数。
    2. 采样坐标，uv 方向上的采样点是一样多的，只是间距不同。跟我上面理解的 anisotropic filtering 有点差别 (越狭长采样点越多)，这样的开销会不会很大？如果只采样 $(u plus.minus k, v plus.minus l)$ 这样的在采样范围内的“格点”，是否有问题？
  ]
]]

问题：能处理水平方向或竖直方向狭长的 footprint，但对角狭长的 footprint 还是个问题。

=== EWA Filtering
EWA = Elliptically Weighted Average，用椭圆来近似 footprint，以 $bold(r_1), bold(r_2)$ 确定椭圆的两条轴。详见 #link("https://zhuanlan.zhihu.com/p/105167411")。
#rect(width: 100%, fill: luma(240), stroke: (left: 1.5pt + black))[#text(0.8em)[
- 取 $D=min{bold(r_1), bold(r_2)}$，取出对应的 mipmap。
- 将圆 $x^2+y^2=1$ 变换到以 $bold(r_1)=(u_x, v_x), bold(r_2)=(u_y, v_y)$ 为基的坐标系下，得到目标椭圆。
  #rect(stroke: (left: 1pt + luma(120)))[
    令 $bold(p)=(x, y)^T$，则圆方程为 $bold(p)^T bold(p)=1$。变换矩阵为 $T=display(mat(u_x, u_y; v_x, v_y))$，由 $bold(p)'=T bold(p) ==> bold(p)=T^(-1) bold(p)'$，代入原方程，有 $bold(p)'^T (T^(-1))^T T^(-1) bold(p)'=1$，展开即得椭圆方程 $A'x^2+B'x y+C'y^2=1$，其中 $ A'=(v_x^2+v_y^2)/(u_x v_y-u_y v_x)^2, quad B'=(-2(u_x v_x+u_y v_y))/(u_x v_y-u_y v_x)^2, quad C'= (u_x^2+u_y^2)/(u_x v_y-u_y v_x)^2 $
  ]
  #rect(stroke: (left: 1pt + luma(120)))[
    然后计算椭圆的长轴、短轴及旋转角。由标准椭圆 $A x^2+C y^2=1$，记为 $bold(p)^T M bold(p)=1$，其中 $M=display(mat(A,O;O,C))$。作用旋转矩阵 $R=display(mat(cos theta, -sin theta; sin theta, cos theta))$ 得 (同上) $bold(p)^T (R^(-1))^T M R^(-1)=bold(p)^T R M R^T bold(p)=1$，展开得椭圆方程 $A'x^2+B'x y+C'y^2=1$，其中 $A'=A cos^2 theta+C sin^2 theta, quad B'=2(A-C)sin theta cos theta, quad C'=A sin^2 theta+C cos^2 theta$。反解得 $det=(A'-C')^2+B'^2, quad A=1/2(A'+C'-sqrt(det)), quad C=1/2(A'+C'+sqrt(det))$，$ "旋转角" theta=1/2 arctan(B'/(A'-C')), quad "长轴长" a=1/sqrt(A), quad "短轴长" b=1/sqrt(C) $
  ]
- 确定椭圆的 bounding box，然后遍历其中的 texel，对落在椭圆内部的做加权平均 (例如用 Gaussian filter)。文章中用 $max{a cos theta, b sin theta}, max{a sin theta, b cos theta}$ 分别近似计算 uv 轴的正负范围，这个 box 并没有完全包括住椭圆。
  #text(fill: luma(120), font: ("STKaiti"))[注：这里公式的正负号/绝对值这种问题还没想过，不知道有没有问题。]
]]
评论区说 EWA 开销很大。文章 #link("https://zhuanlan.zhihu.com/p/362053970") 提到了几个优化，没看。


= Applications of Texture Mapping (*没有深入理解*)

== Bump Maps & Normal Maps
在 texture 中同时存储表面的法线 normal 信息，来 fake 表面的凹凸感 (实际 3D 形状是没变的，例如还是一个光滑的球面，只是表面上的每个点的法线采用了 texture 给出的“法线”)。

normal 所在的坐标是独立的，它和物体表面信息绑定。实践中取物体表面的切平面，texture coordinate function 导出的分别保持 uv 坐标中 u 或 v 不变的两条线就是切平面上的两个正交向量 (需要正交化)，从而导出整个坐标系。

bump map: 不直接存储 normal 信息，而是存储每个点的高度信息 (local height, 相对于 smooth surface 的高度)。求梯度就是 normal，即 #box(fill: luma(240), outset: 2.9pt, radius: 2pt)[($nabla h(bold(p)),1)$.`norm`]，注意归一化！

问题：在边缘处、阴影处能发现露馅了。

== Displacement Maps
texture 的信息和 bump map 一样，存储高度信息。

实际修改表面信息，将 meshes 的每个 vertex 沿着 smooth surface 的法线方向移动对应距离。

== Shadow Maps
跟 ray tracing 没关系，rasterized rendering 特有的。

确定某处能否被点灯源 (point light source) 照亮。关键观察：能被照亮 ==  能被放在同一位置的 camera 拍到。因此只要预先正常 rasterize 一遍，得到 depth map (camera 在 point light 位置的 z-buffer)。之后在原先的 render 过程中，判断一个点是否被照亮，只要先对其做和 depth map 时同样的 perspective projection，然后 look-up 深度值 $d_"map"$ (这个方向上距光源最近的物体的距离) 并和该点距光源实际距离 d 比较。如果 $d approx d_"map"$ (考虑浮点误差!) 说明能被照亮；$d>d_"map"$ 说明不能照亮。

look up 时用深度插值并没有意义 (在 depth value 突变处)，因此 nearest neighbor 就行，或者 multiple sampless 但不要用 depth 插值，而是 0-1 插值 (照的到/照不到)，更有意义。

== Environment Maps
Introduce detail into the illumination without having to model complicated light  source geometry.

当光源从很远处照来时，可以认为每个点的 illumination 差不多，illumination 只与观察视角有关。因此可建立一张 environment map，定义在一个单位球上 (视角的各个方向)。

简单的应用：ray tracing 时，没有 hit 到物体的光线，我们可以返回一个它的方向的环境光。这样，shiny objects that reflect other scene objects will now also reflect the background environment.
