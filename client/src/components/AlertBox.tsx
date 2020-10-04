import {
  Text,
  Alert as ChakraAlert,
  AlertIcon,
  CloseButton,
  AlertDescription,
} from '@chakra-ui/core';
import { formatDuration, intervalToDuration, addMinutes } from 'date-fns';
import React, { useEffect, useState } from 'react';
import { Alert } from '../types';

interface Props {
  alert: Alert;
  pname?: string;
  remove?: () => void;
}

const AlertBox: React.FC<Props> = ({ alert, pname, remove }: Props) => {
  const isInfo = alert.type === 'rtt';
  const text =
    pname != null
      ? isInfo
        ? `${pname} is ready to take!`
        : `Return your ${pname} bottle to the stand!`
      : isInfo
      ? `Ready to take a prescription!`
      : `A pill bottle was removed from the stand and was not returned`;

  const [timediff, setTimediff] = useState<string>(
    formatDuration(
      intervalToDuration({
        start: addMinutes(alert.time, alert.time.getTimezoneOffset()),
        end: Date.now(),
      })
    )
      .split(' ')
      .slice(0, 2)
      .join(' ')
  );

  useEffect(() => {
    const interval = setInterval(() => {
      const td = formatDuration(
        intervalToDuration({
          start: addMinutes(alert.time, alert.time.getTimezoneOffset()),
          end: Date.now(),
        })
      )
        .split(' ')
        .slice(0, 2)
        .join(' ');
      setTimediff(td);
    }, 1000);

    return (): void => {
      clearInterval(interval);
    };
  }, [alert.time]);

  return (
    <ChakraAlert status={isInfo ? 'success' : 'error'} borderRadius={10}>
      <AlertIcon />
      <AlertDescription display="flex" flexDir="row">
        <Text mr={2}>{text}</Text>
        {timediff && (
          <Text fontStyle="oblique" color="gray.500">
            {timediff} ago
          </Text>
        )}
      </AlertDescription>
      <CloseButton position="absolute" right="8px" top="8px" onClick={remove} />
    </ChakraAlert>
  );
};

export default AlertBox;
