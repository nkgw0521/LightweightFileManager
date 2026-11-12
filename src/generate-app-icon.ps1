Add-Type -AssemblyName System.Drawing

$outputPath = Join-Path $PSScriptRoot "app.ico"
$previewPath = Join-Path $PSScriptRoot "app-icon-preview.png"
$bitmap = [System.Drawing.Bitmap]::new(256, 256, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
$graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality

function New-RoundedPath([single]$x, [single]$y, [single]$width, [single]$height, [single]$radius) {
    $path = [System.Drawing.Drawing2D.GraphicsPath]::new()
    $diameter = 2 * $radius
    $path.AddArc($x, $y, $diameter, $diameter, 180, 90)
    $path.AddArc($x + $width - $diameter, $y, $diameter, $diameter, 270, 90)
    $path.AddArc($x + $width - $diameter, $y + $height - $diameter, $diameter, $diameter, 0, 90)
    $path.AddArc($x, $y + $height - $diameter, $diameter, $diameter, 90, 90)
    $path.CloseFigure()
    return $path
}

function New-FolderPath([single]$offsetY) {
    $path = [System.Drawing.Drawing2D.GraphicsPath]::new()
    $path.StartFigure()
    $path.AddLine(45, 101 + $offsetY, 92, 101 + $offsetY)
    $path.AddLine(112, 120 + $offsetY, 197, 120 + $offsetY)
    $path.AddArc(185, 108 + $offsetY, 26, 26, 270, 90)
    $path.AddLine(211, 121 + $offsetY, 211, 188 + $offsetY)
    $path.AddArc(197, 174 + $offsetY, 28, 28, 0, 90)
    $path.AddLine(211, 202 + $offsetY, 48, 202 + $offsetY)
    $path.AddArc(34, 188 + $offsetY, 28, 28, 90, 90)
    $path.AddLine(34, 202 + $offsetY, 34, 112 + $offsetY)
    $path.AddArc(34, 101 + $offsetY, 22, 22, 180, 90)
    $path.CloseFigure()
    return $path
}

$backgroundPath = New-RoundedPath 8 8 240 240 58
$backgroundBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
    [System.Drawing.Rectangle]::new(8, 8, 240, 240),
    [System.Drawing.Color]::FromArgb(255, 37, 60, 91),
    [System.Drawing.Color]::FromArgb(255, 17, 27, 43),
    [System.Drawing.Drawing2D.LinearGradientMode]::ForwardDiagonal)
$graphics.FillPath($backgroundBrush, $backgroundPath)

$shadowPath = New-FolderPath 7
$shadowBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(85, 3, 9, 18))
$graphics.FillPath($shadowBrush, $shadowPath)

$folderPath = New-FolderPath 0
$folderBrush = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
    [System.Drawing.Rectangle]::new(34, 96, 178, 110),
    [System.Drawing.Color]::FromArgb(255, 112, 184, 255),
    [System.Drawing.Color]::FromArgb(255, 56, 123, 232),
    [System.Drawing.Drawing2D.LinearGradientMode]::ForwardDiagonal)
$graphics.FillPath($folderBrush, $folderPath)

$frontPath = [System.Drawing.Drawing2D.GraphicsPath]::new()
$frontPath.StartFigure()
$frontPath.AddLine(35, 128, 113, 128)
$frontPath.AddLine(131, 139, 211, 139)
$frontPath.AddLine(199, 190, 196, 197)
$frontPath.AddArc(184, 185, 26, 26, 0, 90)
$frontPath.AddLine(197, 211, 48, 211)
$frontPath.AddArc(34, 197, 28, 28, 90, 90)
$frontPath.AddLine(34, 211, 34, 141)
$frontPath.AddArc(34, 128, 26, 26, 180, 90)
$frontPath.CloseFigure()
$frontBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(255, 75, 145, 237))
$graphics.FillPath($frontBrush, $frontPath)

$screenPath = New-RoundedPath 66 148 124 48 10
$screenFill = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(255, 21, 36, 58))
$graphics.FillPath($screenFill, $screenPath)
$screenPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(150, 180, 221, 255), 3)
$graphics.DrawPath($screenPen, $screenPath)
$linePen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(225, 209, 232, 255), 4)
$linePen.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
$linePen.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
$dividerPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(185, 127, 167, 215), 3)
$graphics.DrawLine($dividerPen, 128, 153, 128, 191)
foreach ($y in @(163, 175, 187)) {
    $graphics.DrawLine($linePen, 82, $y, 106, $y)
    $graphics.DrawLine($linePen, 148, $y, 172, $y)
}

$accentPen = [System.Drawing.Pen]::new([System.Drawing.Color]::FromArgb(255, 21, 36, 58), 8)
$accentBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(255, 97, 225, 192))
$graphics.FillEllipse($accentBrush, 181, 183, 30, 30)
$graphics.DrawEllipse($accentPen, 181, 183, 30, 30)

$bitmap.Save($previewPath, [System.Drawing.Imaging.ImageFormat]::Png)
$memory = [System.IO.MemoryStream]::new()
$bitmap.Save($memory, [System.Drawing.Imaging.ImageFormat]::Png)
$png = $memory.ToArray()
$stream = [System.IO.File]::Create($outputPath)
$writer = [System.IO.BinaryWriter]::new($stream)
$writer.Write([UInt16]0)
$writer.Write([UInt16]1)
$writer.Write([UInt16]1)
$writer.Write([Byte]0)
$writer.Write([Byte]0)
$writer.Write([Byte]0)
$writer.Write([Byte]0)
$writer.Write([UInt16]1)
$writer.Write([UInt16]32)
$writer.Write([UInt32]$png.Length)
$writer.Write([UInt32]22)
$writer.Write($png)
$writer.Dispose()
$memory.Dispose()
$stream.Dispose()

$graphics.Dispose()
$bitmap.Dispose()
$backgroundPath.Dispose()
$backgroundBrush.Dispose()
$shadowPath.Dispose()
$shadowBrush.Dispose()
$folderPath.Dispose()
$folderBrush.Dispose()
$frontPath.Dispose()
$frontBrush.Dispose()
$screenPath.Dispose()
$screenFill.Dispose()
$screenPen.Dispose()
$linePen.Dispose()
$dividerPen.Dispose()
$accentPen.Dispose()
$accentBrush.Dispose()

Write-Output "Created $outputPath and $previewPath"
