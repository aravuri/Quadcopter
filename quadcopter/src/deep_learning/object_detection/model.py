from torch import nn


class BasicConv2d(nn.Module):

    def __init__(self, in_channels, out_channels, kernel_size, stride=1, padding=0):
        super().__init__()
        self.conv = nn.Conv2d(in_channels, out_channels, kernel_size=kernel_size, stride=stride,
                              padding=padding, bias=False)
        self.bn = nn.BatchNorm2d(out_channels)
        self.relu = nn.ReLU()

    def forward(self, x):
        x = self.conv(x)
        x = self.bn(x)
        x = self.relu(x)
        return x


class Bottleneck(nn.Module):
    expansion = 4

    def __init__(self, in_channels, planes, layer_num, first_resblock=False):
        super().__init__()
        self.first_resblock = first_resblock
        self.layer_num = layer_num

        if not self.first_resblock:
            self.bn_1 = nn.BatchNorm2d(in_channels)
            self.relu = nn.ReLU()

        self.basic_conv1 = BasicConv2d(in_channels, planes, kernel_size=1, stride=1, padding=0)

        if layer_num == 0:
            self.downsample = nn.Conv2d(in_channels, planes * self.expansion,
                                        kernel_size=3, stride=2, padding=1, bias=False)
            self.basic_conv2 = BasicConv2d(planes, planes, kernel_size=3, stride=2, padding=1)
        else:
            self.basic_conv2 = BasicConv2d(planes, planes, kernel_size=3, stride=1, padding=1)

        self.conv_3 = nn.Conv2d(planes, planes * self.expansion, kernel_size=1, stride=1, padding=0, bias=False)

    def forward(self, x):
        if self.layer_num == 0:
            residual_x = self.downsample(x)
        else:
            residual_x = x

        if not self.first_resblock:
            x = self.bn_1(x)
            x = self.relu(x)

        x = self.basic_conv1(x)
        x = self.basic_conv2(x)
        x = self.conv_3(x)

        x += residual_x
        return x


class ResidualBlock(nn.Module):

    def __init__(self, in_channels, planes, blocks, first_resblock=False):
        super().__init__()
        self.layers = []
        prev_channels = in_channels
        for i in range(blocks):
            self.layers.append(Bottleneck(prev_channels, planes, i, first_resblock=first_resblock))
            first_resblock = False
            prev_channels = planes * self.layers[i].expansion

    def forward(self, x):
        for layer in self.layers:
            x = layer(x)
        return x


class UpSample(nn.Module):

    def __init__(self, deconv_channels, conv_channels, out_channels, in_padding=1, out_padding=1):
        super().__init__()
        self.deconv = nn.ConvTranspose2d(deconv_channels, out_channels, kernel_size=3 + out_padding, stride=2,
                                         padding=in_padding, bias=False)
        self.conv = nn.Conv2d(conv_channels, out_channels, kernel_size=1, stride=1, padding=0, bias=False)
        self.bn = nn.BatchNorm2d(out_channels)
        self.relu = nn.ReLU()

    def forward(self, x):
        deconv = self.deconv(x[0])
        conv = self.conv(x[1])

        x = conv + deconv
        x = self.bn(x)
        x = self.relu(x)
        return x


class Backbone(nn.Module):

    def __init__(self, in_channels):
        super().__init__()
        self.conv_1 = BasicConv2d(in_channels, 32, kernel_size=3, stride=1, padding=1)
        self.conv_2 = BasicConv2d(32, 32, kernel_size=3, stride=1, padding=1)

        self.res_block_2 = ResidualBlock(32, 24, 3, first_resblock=True)

        self.res_block_3 = ResidualBlock(96, 48, 6, first_resblock=False)
        self.res_block_3_bn = nn.BatchNorm2d(192)

        self.res_block_4 = ResidualBlock(192, 64, 6, first_resblock=False)
        self.res_block_4_bn = nn.BatchNorm2d(256)

        self.res_block_5 = ResidualBlock(256, 96, 3, first_resblock=False)
        self.res_block_5_bn = nn.BatchNorm2d(384)

        self.conv_3 = BasicConv2d(384, 196, kernel_size=1, stride=1, padding=0)
        self.upsample_6 = UpSample(196, 256, 128, in_padding=1, out_padding=0)
        self.upsample_7 = UpSample(128, 192, 96, in_padding=1, out_padding=1)

        self.relu = nn.ReLU()

    def forward(self, x):
        x = self.conv_1(x)
        x = self.conv_2(x)

        x = self.res_block_2(x)
        res_3 = self.res_block_3(x)

        res_4 = self.res_block_4(res_3)
        res_4 = self.res_block_4_bn(res_4)
        res_4 = self.relu(res_4)

        res_5 = self.res_block_5(res_4)
        res_5 = self.res_block_5_bn(res_5)
        res_5 = self.relu(res_5)

        x = self.conv_3(res_5)
        x = self.upsample_6([x, res_4])
        x = self.upsample_7([x, res_3])

        return x


class Header(nn.Module):

    def __init__(self, in_channels, out_channels, num_classes, blocks=4):
        super().__init__()
        layers = []
        for i in range(blocks):
            layers.append(BasicConv2d(in_channels if i == 0 else out_channels, out_channels,
                                      kernel_size=3, stride=1, padding=1))

        self.header = nn.Sequential(*layers)
        self.cls = nn.Conv2d(out_channels, num_classes, kernel_size=3, stride=1, padding=1, bias=True)
        self.obj = nn.Conv2d(out_channels, 1, kernel_size=3, stride=1, padding=1, bias=True)
        self.reg = nn.Conv2d(out_channels, 6, kernel_size=3, stride=1, padding=1, bias=True)

    def forward(self, x):
        x = self.header(x)

        cls = self.cls(x)
        obj = self.obj(x)
        reg = self.reg(x)

        return cls, obj, reg


class Pixor(nn.Module):

    def __init__(self, in_channels, num_classes):
        super().__init__()
        self.backbone = Backbone(in_channels)
        self.header = Header(96, 96, num_classes, blocks=4)
        self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        x = self.backbone(x)
        cls, obj, reg = self.header(x)
        obj = self.sigmoid(obj)

        return cls, obj, reg


def criterion(cls, obj, reg, true_cls, true_obj, true_reg):
    pass
