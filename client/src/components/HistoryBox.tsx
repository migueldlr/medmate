import { Box, Heading, Stack, Text } from '@chakra-ui/core';
import { compareAsc, format, isSameDay, subDays } from 'date-fns';
import React, { ReactNode } from 'react';
import { HistoryItem, Prescription } from '../types';

interface Props {
  prescriptions: Prescription[];
  hist: HistoryItem[];
}

interface ColumnProps {
  prescriptions: Prescription[];
  hist: HistoryItem[];
  date: Date;
}

const NUM_DAYS = 5;

const HistoryColumn: React.FC<ColumnProps> = ({
  prescriptions,
  date,
  hist,
}: ColumnProps) => {
  return (
    <Stack spacing={0}>
      <Box h="60px" p={1}>
        <Heading size="md">{format(date, 'E')}</Heading>
        <Heading size="md">{format(date, 'MMM d')}</Heading>
      </Box>
      {prescriptions.map((p, i) => {
        const history = hist.filter(
          (t) => isSameDay(t.time, date) && p._id === t.prescription
        );
        history.sort((a, b) => compareAsc(a.time, b.time));
        return (
          <Box
            key={i}
            h="80px"
            p={1}
            borderTop="1px black inset"
            borderColor="gray.200"
          >
            {history.map((h) => {
              return (
                <Text key={format(h.time, 'h:mm a')}>
                  {format(h.time, 'h:mm a')}
                </Text>
              );
            })}
          </Box>
        );
      })}
    </Stack>
  );
};

const HistoryBox: React.FC<Props> = ({ prescriptions, hist }: Props) => {
  const columns: ReactNode[] = [];
  for (let i = 0; i < NUM_DAYS; i++) {
    columns.unshift(
      <HistoryColumn
        prescriptions={prescriptions}
        hist={hist}
        date={subDays(Date.now(), i)}
      />
    );
  }

  return (
    <Box bg="white" p={6} borderRadius={10}>
      <Box display="grid" gridTemplateColumns={`auto repeat(${NUM_DAYS}, 1fr)`}>
        <Stack spacing={0}>
          <Box
            h="60px"
            borderRight="1px black inset"
            borderColor="gray.200"
          ></Box>
          {prescriptions.map((p) => (
            <Box
              h="80px"
              key={p.name}
              display="flex"
              flexDirection="column"
              justifyContent="center"
              borderTop="1px black inset"
              borderRight="1px black inset"
              borderColor="gray.200"
              textAlign="right"
              pr={1}
            >
              <Heading size="md">{p.name}</Heading>
            </Box>
          ))}
        </Stack>
        {columns}
      </Box>
    </Box>
  );
};

export default HistoryBox;
