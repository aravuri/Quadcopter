import torch
from tqdm import tqdm

from nlp import model


class Train(object):

    def __init__(self, config):
        self.config = config
        self.train_dataloader = torch.utils.data.DataLoader()  # TODO: write dataset and use it in DataLoader
        self.val_dataloader = torch.utils.data.DataLoader()
        self.model = model.Pixor(config.in_channels, config.num_classes)
        self.optimizer = torch.optim.Adam(self.model.parameters(), lr=config.lr, weight_decay=config.weight_decay)
        self.callbacks = None  # TODO: add LR Scheduler
        # TODO: load checkpoint

    def train(self, num_epochs):
        self.callbacks.on_train_begin()
        for epoch in range(num_epochs):
            self.callbacks.on_epoch_begin()

            self.model.train()
            tq = tqdm(self.train_dataloader)
            for batch_data in tq:
                self.callbacks.on_batch_begin()
                x, true_cls, true_obj, true_reg = self._preprocess(batch_data)
                if x is None:
                    continue

                cls, obj, reg = self.model(x)
                loss_cls, loss_obj, loss_reg = model.criterion(cls, obj, reg, true_cls, true_obj, true_reg)
                loss = loss_cls + loss_obj + loss_reg

                self.optimizer.zero_grad()
                loss.backward()
                self.optimizer.step()

                self.callbacks.on_batch_end()  # update LR, momentum and weight decay
                # TODO: publish training metrics

            self.callbacks.on_epoch_end()
            # TODO: save checkpoint
            self.validate()

        self.callbacks.on_train_end()

    def validate(self):
        self.model.eval()
        with torch.no_grad():
            tq = tqdm(self.val_dataloader)
            for batch_data in tq:
                x, true_cls, true_obj, true_reg = self._preprocess(batch_data)
                if x is None:
                    continue

                cls, obj, reg = self.model(x)
                loss_cls, loss_obj, loss_reg = model.criterion(cls, obj, reg, true_cls, true_obj, true_reg)
                loss = loss_cls + loss_obj + loss_reg
                # TODO: publish validation metrics

    @staticmethod
    def _preprocess(batch_data):
        return batch_data["x"], batch_data["cls"], batch_data["obj"], batch_data["reg"]
