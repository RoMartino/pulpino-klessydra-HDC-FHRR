`timescale 1ns / 1ps

module sqrt_tb;

  parameter integer SIZE = 32; // Parameter for the size of the input
  parameter real CLK_PERIOD = 10.0; // Clock period in nanoseconds
  parameter TEST_ALL = 0; // Set to 1 to test all cases up to 2^(SIZE-1), 0 for 1000 random cases

  // Signals
  reg clk_i, rst_ni;
  reg start = 1'b0;
  reg [SIZE-1:0] number = 'b0;
  wire [SIZE-1:0] sqrt_res;
  wire busy, ready;

  // Counters
  integer operation_count = 0;
  integer cycle_count = 0;
  integer total_cycle_count = 0;  // Added to track the total cycle count across all operations

  // UUT Instance
  sqrt_nr 
  #(
    .size       (SIZE)
  )
  uut (
    .clk_i      (clk_i),
    .rst_ni     (rst_ni),
    .start      (start),
    .number     (number),
    .sqrt_res   (sqrt_res),
    .busy       (busy),
    .ready      (ready)
  );

  // Clock generation
  initial begin
    clk_i = 0;
    forever #(CLK_PERIOD/2) clk_i = ~clk_i;
  end

  // Reset process
  initial begin
    rst_ni = 0;
    #(2 * CLK_PERIOD);
    rst_ni = 1;
    #(2 * CLK_PERIOD);
    @(posedge clk_i);
    if (TEST_ALL == 1) begin
      for (number = 0; number < 2**(SIZE-1); number = number + 1) begin
        test_case();
      end
    end else begin
      repeat(10000) begin  // Adjusted to 1000 for consistency with parameter description
        number = $urandom_range(0, 2**(SIZE-1) - 1);
        test_case();
      end
    end
    $display("%0d operations have been successfully tested.", operation_count);
    $display("Average Cycles per Operation: %0d", total_cycle_count / operation_count);  // Display the average cycle count
    $stop; // End simulation after testing
  end

  // Modularized test case procedure
  task test_case;
    begin
      cycle_count = 0; // Reset cycle counter for each operation
      start = 1'b1;
      @(posedge clk_i); // Ensure start is registered
      start = 1'b0;
      @(posedge clk_i); // Allow ready to potentially go low
      while (ready != 1'b1) begin
        @(posedge clk_i);
        cycle_count = cycle_count + 1; // Continue incrementing cycle counter
      end
      // Verification and display after ready is asserted
      if (!check_sqrt(number, sqrt_res)) begin
        @(posedge clk_i); // Count the cycle when ready is first seen
        cycle_count = cycle_count + 1;
        total_cycle_count = total_cycle_count + cycle_count; // Accumulate total cycle count
        $display("FAIL: sqrt(%0d) = %0d, Cycles: %0d", number, sqrt_res, cycle_count);
        $error("Test failed for input %0d. Expected sqrt property not met.", number);
        $stop;
      end else begin
        @(posedge clk_i); // Count the cycle when ready is first seen
        cycle_count = cycle_count + 1;
        total_cycle_count = total_cycle_count + cycle_count; // Accumulate total cycle count
        $display("PASS: sqrt(%0d) = %0d, Cycles: %0d", number, sqrt_res, cycle_count);
      end
      operation_count = operation_count + 1;
    end
  endtask

  // Function to check the square root result
  function automatic bit check_sqrt(input integer input_val, input integer output_val);
    check_sqrt = (output_val * output_val <= input_val) && ((output_val + 1) * (output_val + 1) > input_val);
  endfunction

endmodule
