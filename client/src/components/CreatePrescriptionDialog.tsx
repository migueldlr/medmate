import {
  AlertDialog,
  AlertDialogBody,
  AlertDialogContent,
  AlertDialogFooter,
  AlertDialogHeader,
  AlertDialogOverlay,
  Box,
  Button,
  CloseButton,
  FormControl,
  FormErrorMessage,
  FormLabel,
  Heading,
  IconButton,
  Input,
  InputGroup,
  InputRightAddon,
  InputRightElement,
  Text,
} from '@chakra-ui/core';
import { format, parse } from 'date-fns';
import React, { useRef, useState } from 'react';
import { user } from '../firebase/firebase';

interface Props {
  isOpen: boolean;
  onClose: () => void;
}

interface ErrorState {
  name?: string;
  device?: string;
  amount?: string;
}

const CreatePrescriptionDialog: React.FC<Props> = ({
  isOpen,
  onClose,
}: Props) => {
  const [name, setName] = useState('');
  const [device, setDevice] = useState('');
  const [amount, setAmount] = useState<number>();
  const [currTime, setCurrTime] = useState<Date | null>(null);
  const [times, setTimes] = useState<Date[]>([]);
  const [errors, setErrors] = useState<ErrorState>({});
  const [loading, setLoading] = useState(false);

  const validate = (): boolean => {
    const e: ErrorState = {};
    if (name.length === 0) e.name = 'Medicine name is missing';
    if (device.length === 0) e.device = 'Device name is missing';
    if (amount == null) {
      e.amount = 'Medicine amount is missing';
    } else if (amount < 1) {
      e.amount = 'Medicine amount must be greater than 0';
    }

    setErrors(e);
    return e.amount == null && e.name == null && e.device == null;
  };

  const cancelRef = useRef(null);

  const handleSubmit = async (): Promise<void> => {
    if (validate()) {
      setLoading(true);
      user.collection('prescriptions').add({
        amt: amount,
        name,
        device,
        times: times.map((t) => format(t, 'HHmm')),
        step: 1,
        u: 'pill',
        // eslint-disable-next-line @typescript-eslint/camelcase
        a_date: format(Date.now(), 'yyyyMMdd'),
        // eslint-disable-next-line @typescript-eslint/camelcase
        a_time: format(Date.now(), 'HHmm'),
        // eslint-disable-next-line @typescript-eslint/camelcase
        is_pres: times.length > 0,
      });
      onClose();
    }
  };

  return (
    <AlertDialog
      leastDestructiveRef={cancelRef}
      isOpen={isOpen}
      onClose={onClose}
    >
      <AlertDialogOverlay />
      <AlertDialogContent borderRadius={10}>
        <AlertDialogHeader>
          <Heading size="md">Add Prescription</Heading>
        </AlertDialogHeader>
        <AlertDialogBody>
          <FormControl isRequired isInvalid={errors.name != null} mb={2}>
            <FormLabel htmlFor="medicine">Medicine Name</FormLabel>
            <Input
              type="text"
              id="medicine"
              placeholder="Ibuprofen"
              value={name}
              onChange={(event: React.ChangeEvent<HTMLInputElement>): void => {
                setName(event.target.value);
                setErrors({ ...errors, name: undefined });
              }}
            />
            <FormErrorMessage>{errors.name}</FormErrorMessage>
          </FormControl>
          <FormControl isRequired isInvalid={errors.amount != null} mb={2}>
            <FormLabel htmlFor="amount">Dosage</FormLabel>
            <InputGroup>
              <Input
                type="number"
                id="amount"
                placeholder="2"
                value={amount ?? ''}
                onChange={(
                  event: React.ChangeEvent<HTMLInputElement>
                ): void => {
                  const parsed = event.target.valueAsNumber;
                  setAmount(isNaN(parsed) ? undefined : parsed);
                  setErrors({ ...errors, amount: undefined });
                }}
              />
              <InputRightAddon>pill{amount === 1 ? '' : 's'}</InputRightAddon>
            </InputGroup>
            <FormErrorMessage>{errors.amount}</FormErrorMessage>
          </FormControl>
          <FormControl isRequired mb={2}>
            <FormLabel htmlFor="times">Times</FormLabel>
            <InputGroup>
              <Input
                type="time"
                id="times"
                value={currTime != null ? format(currTime, 'HH:mm') : ''}
                onChange={(
                  event: React.ChangeEvent<HTMLInputElement>
                ): void => {
                  const parsed = parse(event.target.value, 'HH:mm', new Date());
                  setCurrTime(isNaN(parsed.getTime()) ? null : parsed);
                }}
              />
              {currTime != null &&
                !times.find(
                  (time) => time.getTime() === currTime.getTime()
                ) && (
                  <InputRightElement>
                    <IconButton
                      aria-label="add time"
                      icon="add"
                      onClick={(): void => {
                        if (currTime == null) return;
                        setTimes([...times, currTime]);
                        setCurrTime(null);
                      }}
                    />
                  </InputRightElement>
                )}
            </InputGroup>
            {
              <Box display="flex">
                {times.map((time, i) => (
                  <Box
                    key={i}
                    bg="gray.200"
                    pl={3}
                    pr={1}
                    py={1}
                    mt={2}
                    borderRadius="0.25rem"
                    display="flex"
                  >
                    <Text mr={1}>{format(time, 'h:mm a')}</Text>
                    <CloseButton
                      size="sm"
                      onClick={(): void => {
                        const timesCopy = [...times];
                        timesCopy.splice(i, 1);
                        setTimes(timesCopy);
                      }}
                    />
                  </Box>
                ))}
              </Box>
            }
          </FormControl>
          <FormControl isRequired isInvalid={errors.device != null} mb={2}>
            <FormLabel htmlFor="device">Device ID</FormLabel>
            <Input
              type="text"
              id="device"
              placeholder="ESP32Test"
              value={device}
              onChange={(event: React.ChangeEvent<HTMLInputElement>): void => {
                setDevice(event.target.value);
                setErrors({ ...errors, device: undefined });
              }}
            />
            <FormErrorMessage>{errors.device}</FormErrorMessage>
          </FormControl>
        </AlertDialogBody>
        <AlertDialogFooter>
          <Button ref={cancelRef} onClick={onClose}>
            Cancel
          </Button>
          <Button
            variantColor="blue"
            onClick={handleSubmit}
            ml={3}
            isLoading={loading}
          >
            Submit
          </Button>
        </AlertDialogFooter>
      </AlertDialogContent>
    </AlertDialog>
  );
};

export default CreatePrescriptionDialog;
